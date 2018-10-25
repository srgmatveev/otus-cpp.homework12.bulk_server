#include "gmock/gmock.h"
#include <thread>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
#include <atomic>
#include <chrono>
#include <vector>
#include <algorithm>
#ifdef WINDOWS
#include <direct.h>
#else
#include <unistd.h>
#endif

using namespace testing;
using namespace std::literals;
using namespace boost::asio;

class TestServer : public Test
{
  public:
    boost::filesystem::path full_path;
    std::string srv_run_path{};
    std::string srv_name = "bulk_server";
    std::string port = "9000";
    std::string bulks = "3";
    std::atomic_bool isServerRunning{false};
    std::thread serverThread;
    std::string out_file = "out.txt";
    void SetUp() override
    {
        isServerRunning = false;
        srv_run_path = "";
        chdir("../");
        full_path = boost::filesystem::current_path();
        full_path += "/" + srv_name;
        if (!boost::filesystem::exists(full_path))
            full_path = "";

        if (!full_path.empty())
        {
            srv_run_path = full_path.string() + " " + port + " " + bulks;
            serverThread = std::thread(&TestServer::startServer, this, srv_run_path, out_file, std::ref(isServerRunning));
            while (!isServerRunning)
            {
            };
            std::this_thread::sleep_for(200ms);
        }
        //    serverThread =
    }
    void TearDown() override
    {
        std::this_thread::sleep_for(200ms);
        std::system("killall bulk_server");
        if (serverThread.joinable())
            serverThread.join();
    }
    void startServer(std::string srv_path, std::string out_file, std::atomic_bool &flag)
    {
        if (!out_file.empty())
            srv_path += " >" + out_file;
        flag = true;
        try
        {
            std::system(srv_path.c_str());
        }
        catch (...)
        {
        }
    }
};

TEST_F(TestServer, TestServer_Is_Running)
{
    std::vector<std::string> str_vec = {
        "cmd1\n", "{\n", "cmd2\n", "cmd3\n",
        "}\n",
        "cmd4\n", "cmd5\n", "cmd6\n"};
    ASSERT_TRUE(srv_run_path.empty() ? 0 : 1);
    boost::asio::io_service io_service;
    ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 9000);
    ip::tcp::socket sock(io_service);
    sock.connect(ep);
    std::for_each(std::cbegin(str_vec),
                  std::cend(str_vec),
                  [&sock](const auto &item) {
                      sock.write_some(buffer(item));
                  });
    sock.close();
}