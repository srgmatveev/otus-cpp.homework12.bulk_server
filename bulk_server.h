#pragma once
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
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
  GetFromClient(boost::asio::io_service &io_service) : sock_(io_service), started_(false)
  {
  }

public:
  static ptr new_(boost::asio::io_service &io_service)
  {
    ptr new_(new GetFromClient(io_service));
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

      //       std::string msg(read_buffer_, bytes);
      // echo message back, and then stop
      //       do_write(msg + "\n");
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
};

class BulkServer;
using server_ptr = boost::shared_ptr<BulkServer>;
class BulkServer : public boost::enable_shared_from_this<BulkServer>, boost::noncopyable
{
  using self_type = BulkServer;

public:
  BulkServer(unsigned short port_number, std::size_t chunk_size) : acceptor(service, ip::tcp::endpoint{ip::tcp::v4(), port_number}), isStarted(false)
  {
  }
  void start()
  {
    if (isStarted)
      return;
    isStarted = true;
    GetFromClient::ptr client = GetFromClient::new_(service);
    acceptor.async_accept(client->sock(), MEM_FN2(handle_accept, client, _1));
    service.run();
  }

  static server_ptr createServer(unsigned short port_number, std::size_t chunk_size)
  {
    return boost::make_shared<BulkServer>(port_number, chunk_size);
  }

private:
  void handle_accept(GetFromClient::ptr client, const boost::system::error_code &err)
  {
    client->start();
    auto new_client = GetFromClient::new_(service);
    acceptor.async_accept(new_client->sock(), MEM_FN2(handle_accept, new_client, _1));
  }

private:
  io_service service;
  ip::tcp::acceptor acceptor;
  bool isStarted;
};