#pragma once
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <iostream>
#include <memory>
#include "bulk.h"

using namespace boost::asio;
#define MEM_FN(x) boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x, y) boost::bind(&self_type::x, shared_from_this(), y)
#define MEM_FN2(x, y, z) boost::bind(&self_type::x, shared_from_this(), y, z)
class GetFromClient : public boost::enable_shared_from_this<GetFromClient>, boost::noncopyable
{
  using self_type = GetFromClient;
  using error_code = boost::system::error_code;

public:
  using ptr = boost::shared_ptr<GetFromClient>;

private:
  GetFromClient(boost::asio::io_service &io_service, size_t chunk_size,
                std::shared_ptr<BulkReadCmd> readCmd,
                std::shared_ptr<ToConsolePrint> consolePrint,
                std::shared_ptr<ToFilePrint> filePrint) : sock_(io_service), started_(false)
  {
    ptrBulkReadCmds = readCmd;
    ptrToConsolePrint = consolePrint;
    ptrToFilePrint = filePrint;
    ptrBulkReadBlock = BulkReadCmd::create(chunk_size);
    if (ptrToConsolePrint)
      ptrBulkReadBlock->subscribe(ptrToConsolePrint);
    if (ptrToFilePrint)
      ptrBulkReadBlock->subscribe(ptrToFilePrint);
  }

public:
  static ptr new_(boost::asio::io_service &io_service, size_t chunk_size,
                  std::shared_ptr<BulkReadCmd> readCmd,
                  std::shared_ptr<ToConsolePrint> consolePrint,
                  std::shared_ptr<ToFilePrint> filePrint)
  {
    ptr new_(new GetFromClient(io_service, chunk_size, readCmd, consolePrint, filePrint));
    return new_;
  }

  void start()
  {
    if (started_)
      return;
    std::cout << "start new client" << std::endl;
    started_ = true;
    do_read();
  }

  void stop()
  {
    if (!started_)
      return;
    if (ptrToConsolePrint)
      ptrBulkReadBlock->unsubscribe(ptrToConsolePrint);
    if (ptrToFilePrint)
      ptrBulkReadBlock->unsubscribe(ptrToFilePrint);
    started_ = false;
    sock_.close();
  }
  ip::tcp::socket &sock() { return sock_; }
  ~GetFromClient() { stop(); }

private:
  void on_read(const error_code &err, size_t bytes)
  {
    if (!err)
    {
      std::istream in(&buffer);
      std::string tmp_str;
      std::getline(in, tmp_str);
      ptrBulkReadBlock->process(tmp_str);
      do_read();
    }
    stop();
  }

  void do_read()
  {
    async_read_until(sock_, buffer, '\n', MEM_FN2(on_read, _1, _2));
  }

private:
  ip::tcp::socket sock_;
  bool started_;
  streambuf buffer;
  std::shared_ptr<BulkReadCmd> ptrBulkReadBlock;
  std::shared_ptr<BulkReadCmd> ptrBulkReadCmds;
  std::shared_ptr<ToConsolePrint> ptrToConsolePrint;
  std::shared_ptr<ToFilePrint> ptrToFilePrint;
};

class BulkServer;
using server_ptr = boost::shared_ptr<BulkServer>;
class BulkServer : public boost::enable_shared_from_this<BulkServer>, boost::noncopyable
{
  using self_type = BulkServer;

public:
  BulkServer(unsigned short port_number, std::size_t chunk_size) : acceptor(service, ip::tcp::endpoint{ip::tcp::v4(), port_number}),
                                                                   isStarted_(false), chunkSize_(chunk_size)
  {
    ptrBulkReadCmds = BulkReadCmd::create(chunk_size);
    ptrToConsolePrint = ToConsolePrint::create(std::cout, ptrBulkReadCmds);
    ptrToFilePrint = ToFilePrint::create(ptrBulkReadCmds, 2);
  }
  void start()
  {
    if (isStarted_)
      return;
    isStarted_ = true;
    GetFromClient::ptr client = GetFromClient::new_(service, chunkSize_,
                                                    ptrBulkReadCmds, ptrToConsolePrint, ptrToFilePrint);
    acceptor.async_accept(client->sock(), MEM_FN2(handle_accept, client, _1));
    service.run();
  }

  void stop()
  {
    if (!isStarted_)
      return;
    service.stop();
    isStarted_ = false;
    if (ptrToConsolePrint && ptrBulkReadCmds)
      ptrToConsolePrint->unsubscribe_on_observable(ptrBulkReadCmds);
    if (ptrToFilePrint && ptrBulkReadCmds)
      ptrToFilePrint->unsubscribe_on_observable(ptrBulkReadCmds);
  }
  static server_ptr createServer(unsigned short port_number, std::size_t chunk_size)
  {
    return boost::make_shared<BulkServer>(port_number, chunk_size);
  }
  ~BulkServer() { stop(); }

private:
  void handle_accept(GetFromClient::ptr client, const boost::system::error_code &err)
  {
    client->start();
    auto new_client = GetFromClient::new_(service, chunkSize_,
                                          ptrBulkReadCmds, ptrToConsolePrint, ptrToFilePrint);
    acceptor.async_accept(new_client->sock(), MEM_FN2(handle_accept, new_client, _1));
  }

private:
  io_service service;
  ip::tcp::acceptor acceptor;
  bool isStarted_;
  size_t chunkSize_;
  std::shared_ptr<BulkReadCmd> ptrBulkReadCmds;
  std::shared_ptr<ToConsolePrint> ptrToConsolePrint;
  std::shared_ptr<ToFilePrint> ptrToFilePrint;
};