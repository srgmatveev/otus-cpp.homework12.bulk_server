#pragma once
#include <string>
#include <cstddef>
#include <map>
#include <string.h>
#include <memory>
#include <thread>

static std::string mainThreadName = "main";
struct Metric
{
    std::thread::id thread_id;
    std::size_t str_cnt = 0;
    std::size_t blk_cnt = 0;
    std::size_t cmd_cnt = 0;
    std::string thread_name{};
};

class MetricsCount
{
  public:
    static MetricsCount &Instance()
    {
        static MetricsCount instance;
        return instance;
    }
    void regThread(std::thread::id, const std::string &);
    void cmdsIncr(std::thread::id, std::size_t);
    void blocksIncr(std::thread::id);
    void stringsIncr(std::thread::id);
    void printStatistic();

  private:
    /// Конструктор приложения по умолчанию
    MetricsCount() = default;
    MetricsCount(const MetricsCount &) = delete;
    MetricsCount(MetricsCount &&rhs) = delete;
    MetricsCount &operator=(const MetricsCount &) = delete;

    std::map<std::thread::id, std::shared_ptr<Metric>> metrics;
};

void blocksCmdsIncr(std::thread::id, std::size_t);