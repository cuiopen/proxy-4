//
//            Copyright (c) Marco Amorim 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "log.h"

class session :
        public boost::enable_shared_from_this<session>
{

public:

    typedef boost::shared_ptr<session> ptr;

    typedef std::pair<boost::shared_ptr<uint8_t[]>, size_t> sp_buffer;

    session(
            boost::asio::io_service& io_service,
            const std::string& session_id,
            const std::string& host,
            const std::string& port,
            size_t buffer_size = 4096);

    virtual ~session();

    boost::asio::ip::tcp::socket& get_socket();

    void start();

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
            boost::asio::ip::tcp::socket& to);

    void handle_send(
            const boost::system::error_code& ec,
            size_t bytes_tranferred,
            sp_buffer buffer);

    void hexdump(
            size_t bytes_tranferred,
            sp_buffer buffer);

    logger_type logger_;

    boost::asio::io_service& io_service_;

    boost::asio::ip::tcp::socket client_;

    boost::asio::ip::tcp::socket server_;

    boost::asio::ip::tcp::resolver resolver_;

    boost::asio::ip::tcp::resolver::query to_;

    size_t buffer_size_;

};
