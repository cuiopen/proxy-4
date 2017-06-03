//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <cstdint>

#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "log.h"

namespace net {

class session :
        public boost::enable_shared_from_this<session>
{

public:

    typedef enum status_ {
        ready,
        running,
        stopped
    } status;

    typedef boost::shared_ptr<session> ptr;

    typedef std::pair<boost::shared_ptr<uint8_t[]>, size_t> sp_buffer;

    session(
            const std::string& name,
            boost::asio::io_service& io_service,
            const std::string& session_id,
            const std::string& host,
            const std::string& port,
            size_t buffer_size,
            bool enable_hexdump,
            size_t client_delay,
            size_t server_delay);

    virtual ~session();

    boost::asio::ip::tcp::socket& get_socket();

    void start();

    void stop();

    const std::string& get_id();

protected:

    void handle_resolve(
            const boost::system::error_code& ec,
            boost::asio::ip::tcp::resolver::iterator it);

    void handle_connect(
            const boost::system::error_code& ec);

    void handle_read(
            const boost::system::error_code& ec,
            size_t bytes_tranferred,
            sp_buffer buffer,
            boost::asio::ip::tcp::socket& from,
            boost::asio::ip::tcp::socket& to,
            bool server_flag);

    void handle_send(
            const boost::system::error_code& ec,
            size_t bytes_tranferred,
            sp_buffer buffer);

    void hexdump(
            size_t bytes_tranferred,
            sp_buffer buffer);

    core::logger_type logger_;

    boost::asio::io_service& io_service_;

    boost::asio::ip::tcp::socket client_;

    boost::asio::ip::tcp::socket server_;

    boost::asio::ip::tcp::resolver resolver_;

    boost::asio::ip::tcp::resolver::query to_;

    std::string id_;

    size_t buffer_size_;

    bool hexdump_enabled_;

    uint64_t total_tx_;

    uint64_t total_rx_;

    size_t client_delay_;

    size_t server_delay_;

    status status_;

    boost::mutex mutex_;

};

} // namespace net
