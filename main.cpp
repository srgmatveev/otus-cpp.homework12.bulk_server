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
        if (argc !=3)
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
        
        unsigned short port_number = std::atoi(argv[1]);
        std::size_t chunk_size = std::stoull(argv[2]);
        boost::asio::io_service io_service;
        auto server = BulkServer::createServer(port_number, chunk_size, io_service);
        server->start();
        
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
