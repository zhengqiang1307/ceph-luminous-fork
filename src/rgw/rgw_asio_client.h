// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
#ifndef RGW_ASIO_CLIENT_H
#define RGW_ASIO_CLIENT_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include "include/assert.h"

#include "rgw_client_io.h"

namespace rgw {
namespace asio {

namespace beast = boost::beast;
using parser_type = beast::http::request_parser<beast::http::buffer_body>;

class ClientIO : public io::RestfulClient,
                 public io::BuffererSink {
 protected:
  parser_type& parser;
 private:
  const bool is_ssl;
  using endpoint_type = boost::asio::ip::tcp::endpoint;
  endpoint_type local_endpoint;
  endpoint_type remote_endpoint;

  RGWEnv env;

  rgw::io::StaticOutputBufferer<> txbuf; // 栈上的小内存

 public:
  ClientIO(parser_type& parser, bool is_ssl,
           const endpoint_type& local_endpoint,
           const endpoint_type& remote_endpoint);
  ~ClientIO() override;

  int init_env(CephContext *cct) override; // 初始化环境
  size_t complete_request() override; // 完成请求
  void flush() override; // 下刷
  size_t send_status(int status, const char *status_name) override; // 发送状态
  size_t send_100_continue() override; // 继续
  size_t send_header(const boost::string_ref& name,
                     const boost::string_ref& value) override; // 发生头
  size_t send_content_length(uint64_t len) override; // 发生内容长度
  size_t complete_header() override; // 完成头

  // 发送内容，将内存数据写到socket stream
  size_t send_body(const char* buf, size_t len) override {
    return write_data(buf, len);
  }

  RGWEnv& get_env() noexcept override {
    return env;
  }
};

} // namespace asio
} // namespace rgw

#endif // RGW_ASIO_CLIENT_H
