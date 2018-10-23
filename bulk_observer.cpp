#include <iostream>
#include <memory>
#include <cstddef>
#include <fstream>
#include "bulk_storage.h"
#include "bulk.h"
#include "bulk_observer.h"
#include "metrics.h"

const std::string ToConsolePrint::thread_base_name = "log";
const std::string ToFilePrint::thread_base_name = "file";

void ToPrint::update(BulkStorage &source, std::size_t id)
{
    std::unique_lock<std::mutex> lock{print_mutex};
    data_queue.emplace(pair_type{source.get_timestamp(id), source.get_commands(id)});
    cv_queue.notify_one();
}
void ToPrint::start(std::size_t threads_count = 1)
{
    if (!threads_count)
        threads_count = 1;

    if (threads_count == 1)
    {
        auto createdThread = std::thread(&ToPrint::printOut, this);
        MetricsCount::Instance().regThread(createdThread.get_id(), _thread_base_name);
        print_threads.emplace_back(std::move(createdThread));
    }
    else
    {
        for (std::size_t i = 0; i < threads_count; ++i)
        {
            auto createdThread = std::thread(&ToPrint::printOut, this);
            MetricsCount::Instance().regThread(createdThread.get_id(), _thread_base_name + std::to_string(i + 1));
            print_threads.emplace_back(std::move(createdThread));
        }
    }

#ifdef DEBUG
    std::cout << _thread_base_name << "_log thread created" << console_threads.size() << std::endl;
#endif
}
void ToPrint::stop()
{
    finished = true;
    cv_queue.notify_all();
    if (print_threads.empty())
    {
        return;
    }
    for (auto &thread : print_threads)
        if (thread.joinable())
            thread.join();

#ifdef DEBUG
    std::cout << _thread_base_name << "_log threads stopped" << std::endl;
#endif
    print_threads.clear();
}

void ToPrint::printOut()
{
    while (!finished || !data_queue.empty())
    {
        std::unique_lock<std::mutex> locker{print_mutex};

        while (data_queue.empty() && !finished)
            cv_queue.wait(locker);
        if (!data_queue.empty())
        {
            auto cmd_pair = data_queue.front();
            data_queue.pop();
            if (isConsole)
            {
                blocksCmdsIncr(std::this_thread::get_id(), cmd_pair.second.size());
                printOstream(_out, cmd_pair.second);
            }
            else
            {
                std::ostringstream oss;
                oss << "bulk";
                oss << cmd_pair.first;
                oss << ".log";
                std::string fName = oss.str();
                std::ofstream ofs;
                ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
                try
                {
                    ofs.open(fName, std::ofstream::out | std::ofstream::trunc);
                    locker.unlock();
                    blocksCmdsIncr(std::this_thread::get_id(), cmd_pair.second.size());
                    printOstream(ofs, cmd_pair.second);
                    ofs.close();
                }
                catch (std::ofstream::failure &e)
                {
                    locker.unlock();
                    std::cerr << "Exception opening/reading/closing file: " << fName << std::endl;
                }
            }
        }
    }
}

void Observer::subscribe_on_observable(const std::weak_ptr<Observable> &observable)
{
    auto item = observable.lock();
    if (item)
    {
        auto it = std::find_if(_observables.cbegin(), _observables.cend(), [&](std::weak_ptr<Observable> e) { return e.lock() == item; });
        if (it == _observables.cend())
        {
            _observables.emplace_back(item);
            item->subscribe(this->shared_from_this());
        }
        item.reset();
    }
}

void Observer::unsubscribe_on_observable(const std::weak_ptr<Observable> &observable_ptr)
{
    auto observable = observable_ptr.lock();
    if (observable)
    {
        _observables.erase(
            std::remove_if(_observables.begin(), _observables.end(),
                           [observable_ptr](const auto &p) { return !(observable_ptr.owner_before(p) || p.owner_before(observable_ptr)); }),
            _observables.end());

        observable.reset();
    }
}
