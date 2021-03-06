#pragma once
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <memory>
#include "bulk.h"

using namespace boost::asio;
#define MEM_FN(x) boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x, y) boost::bind(&self_type::x, shared_from_this(), y)
#define MEM_FN2(x, y, z) boost::bind(&self_type::x, shared_from_this(), y, z)
#define MEM_FN3(x, y, w, z) boost::bind(&self_type::x, shared_from_this(), y, w, z)
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
                std::shared_ptr<ToFilePrint> filePrint) : sock_ptr(boost::make_shared<boost::asio::ip::tcp::socket>(io_service)),
                                                          started_(false), countBrackets_{0}
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
    started_ = true;
    do_read();
  }

  void stop()
  {
    if (!started_)
      return;
    if (ptrBulkReadCmds)
      ptrBulkReadCmds->flush();
    if (ptrBulkReadBlock)
      ptrBulkReadBlock->flush();
    if (ptrToConsolePrint)
      ptrBulkReadBlock->unsubscribe(ptrToConsolePrint);
    if (ptrToFilePrint)
      ptrBulkReadBlock->unsubscribe(ptrToFilePrint);
    started_ = false;
    sock_ptr->close();
  }
  ip::tcp::socket &sock() { return *sock_ptr; }
  ~GetFromClient() { stop(); }

private:
  void do_read()
  {
    auto self = shared_from_this();
    async_read_until(*sock_ptr, buffer, '\n',
                     [self](const error_code &error, size_t bytes) {
                       if (!error)
                       {
                         std::istream is(&self->buffer);
                         std::string tmp_str;
                         std::getline(is, tmp_str);
                         if (tmp_str == "{")
                         {
                           ++(self->countBrackets_);
                           self->ptrBulkReadBlock->process(tmp_str);
                         }
                         else if (tmp_str == "}")
                         {
                           self->ptrBulkReadBlock->process(tmp_str);
                           --self->countBrackets_;
                         }
                         else
                         {
                           if (!self->countBrackets_)
                           {
                             self->ptrBulkReadCmds->process(tmp_str);
                           }
                           else
                           {
                             self->ptrBulkReadBlock->process(tmp_str);
                           }
                         }
                         self->do_read();
                         return;
                       }

                       self->stop();
                     });
  }

private:
  boost::shared_ptr<ip::tcp::socket> sock_ptr;
  bool started_;
  streambuf buffer;
  std::shared_ptr<BulkReadCmd> ptrBulkReadBlock;
  std::shared_ptr<BulkReadCmd> ptrBulkReadCmds;
  std::shared_ptr<ToConsolePrint> ptrToConsolePrint;
  std::shared_ptr<ToFilePrint> ptrToFilePrint;
  std::size_t countBrackets_;
};

class BulkServer : public boost::enable_shared_from_this<BulkServer>, boost::noncopyable
{
  using self_type = BulkServer;
  using error_code = boost::system::error_code;

public:
  BulkServer(unsigned short port_number, std::size_t chunk_size,
             boost::asio::io_service &io_service_, bool asker = false) : service(io_service_), acceptor(service, ip::tcp::endpoint{ip::tcp::v4(), port_number}),
                                                                         isStarted_(false), chunkSize_(chunk_size), ask_close(asker)
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

    boost::asio::signal_set signals(service, SIGINT);
    signals.async_wait(MEM_FN3(handler,
                               boost::ref(signals), _1, _2));
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
  static auto createServer(unsigned short port_number, std::size_t chunk_size,
                           boost::asio::io_service &io_service_, bool asker = false)
  {
    return boost::make_shared<BulkServer>(port_number, chunk_size, io_service_, asker);
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
  void handler(boost::asio::signal_set &this_, const error_code &error, int signal_number)
  {
    stop();
  }

private:
  io_service &service;
  ip::tcp::acceptor acceptor;
  bool isStarted_;
  size_t chunkSize_;
  std::shared_ptr<BulkReadCmd> ptrBulkReadCmds;
  std::shared_ptr<ToConsolePrint> ptrToConsolePrint;
  std::shared_ptr<ToFilePrint> ptrToFilePrint;
  bool ask_close;
};