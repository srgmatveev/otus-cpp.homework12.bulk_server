#include <iostream>
#include <cstddef>
#include <memory>
#include <exception>
#include <thread>
#include <string>
#include "bulk.h"
#include "bulk_observer.h"
#include "utils.h"
#include "metrics.h"
#include "bulk_server.h"

int main(int argc, char const *argv[])
{
    try
    {
        if (argc < 3 || argc > 4)
        {
            std::cout << "Launch parameters:  bulk_server <port> <bulk_size>" << std::endl
                      << "where <port> - tcp number port" << std::endl
                      << "<bulk_size> - command block size" << std::endl;
            return 0;
        }
        if (!is_port<char const *>(argv[1]))
        {
            return 0;
        }

        if (!is_numeric<char const *, elem_traits<std::size_t>::value_type>(argv[2]))
        {
            return 0;
        }
        bool asker = false;
        if (argc == 4)
        {
            std::string tempo(argv[3]);
            if (tempo == "y" || tempo == "Y")
                asker = true;
        }
        unsigned short port_number = std::atoi(argv[1]);
        std::size_t chunk_size = std::stoull(argv[2]);
        auto server = BulkServer::createServer(port_number, chunk_size, asker);
        server->start();
        // std::size_t file_threads_count = 2;
        /*  MetricsCount::Instance().regThread(std::this_thread::get_id(), mainThreadName);
        auto ptrBulkRead = BulkReadCmd::create(chunk_size);
        {
            auto ptrToConsolePrint = ToConsolePrint::create(std::cout, ptrBulkRead);
            auto ptrToFilePrint = ToFilePrint::create(ptrBulkRead, file_threads_count);
            ptrBulkRead->process(std::cin);
        }
        MetricsCount::Instance().printStatistic();
        */
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
