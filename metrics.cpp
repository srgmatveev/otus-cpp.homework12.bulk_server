#include <iostream>
#include "metrics.h"

void MetricsCount::regThread(std::thread::id id, const std::string &str)
{
    auto iter = metrics.find(id);
    if (iter == metrics.cend())
    {
        auto metricPtr = std::make_shared<Metric>();
        metricPtr->thread_id = id;
        metricPtr->thread_name = str;
        using type_pair = std::pair<std::thread::id, std::shared_ptr<Metric>>;
        metrics.insert(type_pair{id, metricPtr});
    }
}

void MetricsCount::cmdsIncr(std::thread::id id, std::size_t count)
{
    auto iter = metrics.find(id);
    if (iter != metrics.cend())
    {
        iter->second->cmd_cnt += count;
    }
}

void MetricsCount::blocksIncr(std::thread::id id)
{
    auto iter = metrics.find(id);
    if (iter != metrics.cend())
    {
        ++(iter->second->blk_cnt);
    }
}

void MetricsCount::stringsIncr(std::thread::id id)
{
    auto iter = metrics.find(id);
    if (iter != metrics.cend())
    {
        ++(iter->second->str_cnt);
    }
}

void MetricsCount::printStatistic()
{

    for (const auto &item : metrics)
    {
        if (item.second->thread_name == mainThreadName)
        {
            std::cout << item.second->thread_name
                      << ": lines: " << item.second->str_cnt
                      << ", blocks: " << item.second->blk_cnt
                      << ", cmds: " << item.second->cmd_cnt
                      << std::endl;
        }
        else
        {
            std::cout << item.second->thread_name
                      << ": blocks: " << item.second->blk_cnt
                      << ", cmds: " << item.second->cmd_cnt
                      << std::endl;
        }
    }
}

void blocksCmdsIncr(std::thread::id id, std::size_t count)
{
    MetricsCount::Instance().blocksIncr(id);
    MetricsCount::Instance().cmdsIncr(id, count);
}