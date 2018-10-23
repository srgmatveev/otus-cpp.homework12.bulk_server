#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <memory>
#include <cstddef>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <utility>
#include <thread>
#include "metrics.h"
#include "bulk.h"

//#define DEBUG

class Observable;
class Observer : public std::enable_shared_from_this<Observer>
{
private:
  std::vector<std::weak_ptr<Observable>> _observables;

public:
  virtual void update(BulkStorage &source, std::size_t) = 0;
  void subscribe_on_observable(const std::weak_ptr<Observable> &);
  void unsubscribe_on_observable(const std::weak_ptr<Observable> &);
  virtual void start(std::size_t) = 0;
  virtual void stop() = 0;
  virtual void printOut() = 0;
  virtual ~Observer(){};

protected:
  virtual void printOstream(std::ostream &out, const std::vector<std::string> &vecStr)
  {
    if (!vecStr.empty())
    {
      out << "bulk: ";
      for (const auto &cmd : vecStr)
        out << cmd << (&cmd != &vecStr.back() ? ", " : "");
      out << std::endl;
    }
  }
};

class ToPrint : public Observer
{
private:
  std::mutex print_mutex;
  std::vector<std::thread> print_threads;
  std::ostream &_out{std::cout};
  std::condition_variable cv_queue;
  std::atomic<bool> finished{false};
  std::string _thread_base_name{"log"};
  using pair_type = std::pair<std::size_t, std::vector<std::string>>;
  std::queue<pair_type> data_queue;
  bool isConsole;

public:
  ToPrint(std::ostream &out, const std::string &thread_base_name,
          std::size_t threads_count = 1) : _out{out}, finished{false}, _thread_base_name{thread_base_name},
                                           isConsole{true}
  {
    start(threads_count);
  }

  ToPrint(const std::string &thread_base_name,
          std::size_t threads_count = 1) : finished{false}, _thread_base_name{thread_base_name},
                                           isConsole{false}
  {
    start(threads_count);
  }
  void update(BulkStorage &, std::size_t) override;
  void start(std::size_t) override;
  void stop() override;
  void printOut() override;
  virtual ~ToPrint()
  {
    stop();
  }
};

class ToConsolePrint : public ToPrint
{
private:
  static const std::string thread_base_name;

public:
  ToConsolePrint(std::ostream &out, std::size_t threads_count = 1) : ToPrint(out, thread_base_name, threads_count)
  {
  }
  static auto create(std::ostream &out, const std::weak_ptr<Observable> &_obs, std::size_t threads_count = 1)
  {
    auto _tmpToConsolePrint = std::make_shared<ToConsolePrint>(out, threads_count);
    auto tmpObservable = _obs.lock();
    if (tmpObservable)
    {
      _tmpToConsolePrint->subscribe_on_observable(tmpObservable);
      tmpObservable.reset();
    }
    return _tmpToConsolePrint;
  }
  static auto create(std::ostream &out, std::size_t threads_count = 1)
  {
    return std::make_shared<ToConsolePrint>(out, threads_count);
  }
  virtual ~ToConsolePrint() {}
};

class ToFilePrint : public ToPrint
{
private:
  static const std::string thread_base_name;

public:
  ToFilePrint(std::size_t threads_count = 1) : ToPrint(thread_base_name, threads_count) {}

  static auto create(std::size_t threads_count = 1)
  {
    return std::make_shared<ToFilePrint>(threads_count);
  }

  static auto create(const std::weak_ptr<Observable> &_observable, std::size_t threads_count = 1)
  {
    auto _tmpToFilePrint = std::make_shared<ToFilePrint>(threads_count);
    auto tmpObservable = _observable.lock();
    if (tmpObservable)
    {
      _tmpToFilePrint->subscribe_on_observable(tmpObservable);
      tmpObservable.reset();
    }
    return _tmpToFilePrint;
  }

  virtual ~ToFilePrint()
  {
  }
};