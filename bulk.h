#pragma once
#include <string>
#include <ctime>
#include <algorithm>
#include <memory>
#include <cstddef>
#include <mutex>
#include "utils.h"
#include "bulk_storage.h"
#include "bulk_observer.h"
class Observer;
class Observable
{
  std::mutex lock_mutex;

public:
  void subscribe(const std::weak_ptr<Observer> &observer_ptr)
  {
    auto observer = observer_ptr.lock();

    if (observer)
    {
      auto it = std::find_if(observers.cbegin(), observers.cend(), [&](std::weak_ptr<Observer> e) { return e.lock() == observer; });
      if (it == observers.cend())
        observers.emplace_back(observer);
      observer.reset();
    }
  }
  void unsubscribe(const std::weak_ptr<Observer> &observer_ptr)
  {
    auto observer = observer_ptr.lock();
    if (observer)
    {
      observers.erase(
          std::remove_if(observers.begin(), observers.end(),
                         [observer_ptr](const auto &p) { return !(observer_ptr.owner_before(p) || p.owner_before(observer_ptr)); }),
          observers.end());

      observer.reset();
    }
  }

protected:
  void notify(BulkStorage &source, std::size_t id)
  {
    // std::lock_guard<std::mutex> m_lock(lock_mutex);
    for (const auto &obs : observers)
    {
      if (auto ptr = obs.lock())
      {
        ptr->update(source, id);
        ptr.reset();
      }
    }
    blocksCmdsIncr(std::this_thread::get_id(), source.get_commands(id).size());
  }

private:
  std::vector<std::weak_ptr<Observer>> observers;
};

class BulkReadCmd : public Observable
{
public:
  BulkReadCmd() : Observable(),
                  _chunkSize{1},
                  _bulkStorage{std::make_shared<BulkStorage>()} {}

  BulkReadCmd(const std::size_t &chunkSize) : Observable(),
                                              _chunkSize{chunkSize},
                                              _bulkStorage{std::make_shared<BulkStorage>()} {}

  void process(std::istream &);

  void append(const std::string &);
  void push();

  static std::shared_ptr<BulkReadCmd> create(std::size_t size)
  {
    return std::make_shared<BulkReadCmd>(size);
  }

private:
  std::shared_ptr<BulkStorage> _bulkStorage{nullptr};
  std::size_t _chunkSize{0};
  std::size_t _numb_of_current_chunk{0};
  std::size_t _current_numb_of_cell{0};
  std::size_t open_braces_count{0};
};