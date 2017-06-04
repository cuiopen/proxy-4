//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/thread/thread_pool.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "net/tcp_session.h"
#include "core/log.h"

namespace net {

class tcp_proxy :
        public boost::enable_shared_from_this<tcp_proxy>
{

public:

    typedef boost::shared_ptr<tcp_proxy> ptr;

    typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

    typedef struct config_
    {
        std::string name_;
        std::string shost_;
        std::string sport_;
        std::string dhost_;
        std::string dport_;
        size_t client_delay_;
        size_t server_delay_;
        size_t buffer_size_;
        std::string message_dump_;
    } config;

    tcp_proxy(
            boost::asio::io_service& io_service,
            const config& proxy_config);

    virtual ~tcp_proxy();

    void start();

    void stop();

protected:

    void handle_resolve(
            const boost::system::error_code& ec,
            boost::asio::ip::tcp::resolver::iterator it);

    void handle_accept(
            const boost::system::error_code& ec,
            tcp_session::ptr tcp_session);

    void handle_signal(
            const boost::system::error_code& error,
            int signal_number);

    core::logger_type logger_;

    boost::asio::io_service& io_service_;

    boost::asio::ip::tcp::socket client_;

    boost::asio::ip::tcp::socket server_;

    boost::asio::ip::tcp::acceptor acceptor_;

    boost::asio::ip::tcp::resolver resolver_;

    boost::asio::ip::tcp::resolver::query from_;

    boost::asio::ip::tcp::resolver::query to_;

    std::vector<tcp_session::ptr> sessions_;

    boost::random::random_device random_device_;

    boost::random::uniform_int_distribution<uint32_t> uniform_dist_;

    config config_;
};

} // namespace net
