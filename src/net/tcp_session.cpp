//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <iomanip>
#include <sstream>

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/thread.hpp>

#include "net/tcp_session.h"
using namespace net;

tcp_session::tcp_session(
        boost::asio::io_service& io_service,
        const tcp_session::config& config) :
    logger_(boost::log::keywords::channel =
        std::string("net.tcp_session." + config.type_ + "." + config.id_)),
    io_service_(io_service),
    client_(io_service),
    server_(io_service),
    resolver_(io_service),
    to_(config.host_, config.port_),
    config_(config)
{
    info_.status_ = ready;
    info_.total_tx_ = 0;
    info_.total_rx_ = 0;

    LOG_INFO() << "ctor";
}

tcp_session::~tcp_session()
{
    LOG_INFO() << "dtor";
}

boost::asio::ip::tcp::socket& tcp_session::get_socket()
{
    return server_;
}

void tcp_session::start()
{
    LOG_INFO() << "started";

    info_.start_time_ = boost::chrono::system_clock::now();
    info_.status_ = running;

    resolver_.async_resolve(
                to_,
                boost::bind(
                    &tcp_session::handle_resolve,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::iterator)
                );
}

const std::string& tcp_session::get_id()
{
    return config_.id_;
}

void tcp_session::handle_resolve(
        const boost::system::error_code& ec,
        boost::asio::ip::tcp::resolver::iterator it)
{
    if (!ec)
    {
        boost::asio::ip::tcp::resolver::iterator end;
        if (it != end)
        {
            boost::asio::ip::tcp::endpoint ep(*it);

            LOG_DEBUG() << "to endpoint=["
                        << ep.address() << ":" << ep.port() << "]";

            client_.async_connect(
                        ep,
                        boost::bind(
                            &tcp_session::handle_connect,
                            this,
                            boost::asio::placeholders::error));
        }
    }
    else
    {
        LOG_ERROR() << "ec=[" << ec << "] message=[" << ec.message() << "]";
    }

}

void tcp_session::handle_connect(
        const boost::system::error_code& ec)
{
    if (!ec)
    {
        LOG_DEBUG() << "connected";

        try
        {
            sp_buffer buffer =
                    std::make_pair(
                        boost::make_shared<uint8_t[]>(config_.buffer_size_),
                        config_.buffer_size_);

            client_.async_read_some(
                        boost::asio::buffer(
                            buffer.first.get(), buffer.second),
                        boost::bind(
                            &tcp_session::handle_read, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred,
                            buffer,
                            boost::ref(client_),
                            boost::ref(server_),
                            true));

            buffer =
                    std::make_pair(
                        boost::make_shared<uint8_t[]>(config_.buffer_size_),
                        config_.buffer_size_);

            server_.async_read_some(
                        boost::asio::buffer(
                            buffer.first.get(), buffer.second),
                        boost::bind(
                            &tcp_session::handle_read, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred,
                            buffer,
                            boost::ref(server_),
                            boost::ref(client_),
                            false));
        }
        catch (std::exception& e)
        {
            LOG_ERROR() << "std::exception what=[" << e.what() << "]";
        }
    }
    else
    {
        LOG_ERROR() << " ec=[" << ec << "] message=[" << ec.message() << "]";
    }
}

void tcp_session::hexdump(
        size_t bytes_tranferred,
        sp_buffer buffer)
{
    size_t i = 0;
    uint32_t address = 0;
    std::ostringstream hex_out, ascii_out;

    hex_out << std::setfill('0') << std::setw(8) << address << "    ";

    while (i < bytes_tranferred)
    {
        hex_out << std::hex << std::setw(2)
                << static_cast<uint16_t>(buffer.first[i]) << ' ';

        ascii_out << static_cast<char>(
                         std::isgraph(buffer.first[i]) ? buffer.first[i] : '.');

        ++i;

        if (!(i % 16))
        {
            LOG_DEBUG() << hex_out.str() << "   " << ascii_out.str();

            ascii_out.str("");
            hex_out.str("");

            address += 16;
            hex_out << std::setw(8) << address << "    ";
        }
    }

    if (i % 16)
    {
        while (i++ % 16)
        {
            hex_out << "   ";
        }

        LOG_DEBUG() << hex_out.str() << "   " << ascii_out.str();
    }
}

void tcp_session::stop()
{
    boost::lock_guard<boost::mutex> lock(mutex_);

    if (info_.status_ != stopped)
    {
        server_.close();
        client_.close();
        info_.status_ = stopped;

        info_.stop_time_ = boost::chrono::system_clock::now();

        LOG_INFO() << "stopped tx=[" << info_.total_tx_ << "] "
                   << "rx=[" << info_.total_rx_ << "] "
                   << "elapsed=[" << boost::chrono::duration_cast<
                      boost::chrono::milliseconds>(
                          info_.stop_time_ - info_.start_time_)
                   << "]";
    }

    LOG_DEBUG() << "session stopped";
}

void tcp_session::handle_read(
        const boost::system::error_code& ec,
        size_t bytes_tranferred,
        sp_buffer buffer_read,
        boost::asio::ip::tcp::socket& from,
        boost::asio::ip::tcp::socket& to,
        bool server_flag)
{
    if (!ec && bytes_tranferred)
    {
        try
        {
            to.async_send(
                        boost::asio::buffer(
                            buffer_read.first.get(),
                            bytes_tranferred),
                        boost::bind(
                            &tcp_session::handle_send,
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred,
                            buffer_read));

            if (server_flag)
            {
                boost::lock_guard<boost::mutex> lock(mutex_);

                info_.total_rx_ += bytes_tranferred;

                LOG_DEBUG() << "server=[" << from.local_endpoint().address()
                            << ":" << from.local_endpoint().port() << "] -> "
                            << "client=[" << to.remote_endpoint().address()
                            << ":" << to.remote_endpoint().port() << "] "
                            << "bytes=[" << bytes_tranferred << "]";

                if (config_.server_delay_)
                    boost::this_thread::sleep_for(
                                boost::chrono::microseconds(
                                    config_.server_delay_));
            }
            else
            {
                boost::lock_guard<boost::mutex> lock(mutex_);

                info_.total_tx_ += bytes_tranferred;

                LOG_DEBUG() << "client=[" << from.local_endpoint().address()
                            << ":" << from.local_endpoint().port() << "] -> "
                            << "server=[" << to.remote_endpoint().address()
                            << ":" << to.remote_endpoint().port() << "] "
                            << "bytes=[" << bytes_tranferred << "]";

                if (config_.client_delay_)
                    boost::this_thread::sleep_for(
                                boost::chrono::microseconds(
                                    config_.client_delay_));
            }

            if (config_.message_dump_ == hex)
            {
                hexdump(bytes_tranferred, buffer_read);
            }
            else if (config_.message_dump_ == ascii)
            {
                LOG_DEBUG() << "message=[" << buffer_read.first.get() << "]";
            }

            sp_buffer buffer =
                    std::make_pair(
                        boost::make_shared<uint8_t[]>(config_.buffer_size_),
                        config_.buffer_size_);

            from.async_read_some(
                        boost::asio::buffer(
                            buffer.first.get(), buffer.second),
                        boost::bind(
                            &tcp_session::handle_read, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred,
                            buffer,
                            boost::ref(from),
                            boost::ref(to),
                            server_flag));
        }
        catch (std::exception& e)
        {
            LOG_ERROR() << "std::exception what=[" << e.what() << "]";
        }
    }
    else
    {
        if (!ec)
        {
            LOG_ERROR() << "ec=[" << ec << "] message=[" << ec.message() << "]";
        }
        else
        {
            LOG_DEBUG() << "connection closed - "
                       << (server_flag ? "server" : "client");
        }

        stop();
   }

}

void tcp_session::handle_send(
        const boost::system::error_code& ec,
        size_t bytes_tranferred,
        sp_buffer)
{
    LOG_TRACE() << "bytes sent: " << bytes_tranferred;

    if (ec)
    {
        LOG_ERROR() << "ec=[" << ec << "] message=[" << ec.message() << "]";
    }
}
