//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <iomanip>
#include <sstream>
#include <cctype>

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/hex.hpp>

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
    timeout_timer_(io_service),
    server_timer_(io_service),
    client_timer_(io_service),
    config_(config)
{
    info_.status_ = ready;
    info_.total_tx_ = 0;
    info_.total_rx_ = 0;

    LOG_TRACE() << "ctor";
}

tcp_session::~tcp_session()
{
    LOG_TRACE() << "dtor";
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

    if (config_.timeout_)
        set_timeout(config_.timeout_);
}

void tcp_session::set_timeout(
        uint64_t timeout)
{
    LOG_DEBUG() << "session timeout=[" << config_.timeout_ << "]";

    timeout_timer_.expires_from_now(
                boost::posix_time::microseconds(config_.timeout_));

    timeout_timer_.cancel();

    timeout_timer_.async_wait(
                boost::bind(
                    &tcp_session::handle_timeout,
                    this,
                    boost::asio::placeholders::error));
}

const std::string& tcp_session::get_id()
{
    return config_.id_;
}

const tcp_session::info& tcp_session::get_info()
{
    return info_;
}

void tcp_session::handle_resolve(
        const boost::system::error_code& error_code,
        boost::asio::ip::tcp::resolver::iterator it)
{
    if (!error_code)
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
        LOG_ERROR() << "ec=[" << error_code << "] message=["
                    << error_code.message() << "]";
    }

}

void tcp_session::handle_timeout(
        const boost::system::error_code& error_code)
{
    if (!error_code)
    {
        LOG_WARNING() << "timed out";

        stop();
    }
    else
    {
        LOG_ERROR() << " ec=[" << error_code << "] message=["
                    << error_code.message() << "]";
    }
}

void tcp_session::handle_connect(
        const boost::system::error_code& error_code)
{
    if (!error_code)
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
        LOG_ERROR() << " ec=[" << error_code << "] message=["
                    << error_code.message() << "]";
    }
}

void tcp_session::hexdump(
        const uint8_t* buffer,
        size_t size)
{
    size_t i = 0;
    uint32_t address = 0;
    std::ostringstream out, hex_out, ascii_out;

    hex_out << '\n' << std::setfill('0') << std::setw(8) << address << "    ";

    while (i < size)
    {
        hex_out << std::hex << std::setw(2)
                << static_cast<uint16_t>(buffer[i]) << ' ';

        ascii_out << static_cast<char>(
                         std::isgraph(buffer[i]) ? buffer[i] : '.');

        ++i;

        if (!(i % 16))
        {
            out << hex_out.str() << "   " << ascii_out.str() << '\n';

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

        out << hex_out.str() << "   " << ascii_out.str();
    }

    LOG_DEBUG() << out.str();
}

void tcp_session::stop()
{
    boost::lock_guard<boost::mutex> lock(mutex_);

    if (info_.status_ != stopped)
    {
        timeout_timer_.cancel();
        server_timer_.cancel();
        client_timer_.cancel();
        server_.close();
        client_.close();
        info_.status_ = stopped;

        info_.stop_time_ = boost::chrono::system_clock::now();

        LOG_INFO() << "stats tx=[" << info_.total_tx_ << "] "
                   << "rx=[" << info_.total_rx_ << "] "
                   << "elapsed=[" << boost::chrono::duration_cast<
                      boost::chrono::milliseconds>(
                          info_.stop_time_ - info_.start_time_)
                   << "]";

        LOG_DEBUG() << "stopped";

        signal_stopped_(shared_from_this());
    }
}

void tcp_session::handle_read(
        const boost::system::error_code& error_code,
        size_t bytes_transferred,
        sp_buffer buffer_read,
        boost::asio::ip::tcp::socket& from,
        boost::asio::ip::tcp::socket& to,
        bool server_flag)
{
    if (!error_code && bytes_transferred)
    {
        try
        {
            if (config_.timeout_)
                set_timeout(config_.timeout_);

            to.async_send(
                        boost::asio::buffer(
                            buffer_read.first.get(),
                            bytes_transferred),
                        boost::bind(
                            &tcp_session::handle_send,
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred,
                            buffer_read));

            if (server_flag)
            {
                boost::lock_guard<boost::mutex> lock(mutex_);

                info_.total_rx_ += bytes_transferred;


                LOG_DEBUG() << "server=[" << from.local_endpoint().address()
                            << ":" << from.local_endpoint().port() << "/"
                            << (from.local_endpoint().address().is_v4() ?
                                    "ipv4" : "ipv6") << "] -> "
                            << "client=[" << to.remote_endpoint().address()
                            << ":" << to.remote_endpoint().port() << "/"
                            << (to.local_endpoint().address().is_v4() ?
                                    "ipv4" : "ipv6") << "] "
                            << "bytes=[" << bytes_transferred << "]";

                if (config_.server_delay_)
                    boost::this_thread::sleep_for(
                                boost::chrono::microseconds(
                                    config_.server_delay_));
            }
            else
            {
                boost::lock_guard<boost::mutex> lock(mutex_);

                info_.total_tx_ += bytes_transferred;

                LOG_DEBUG() << "client=[" << from.local_endpoint().address()
                            << ":" << from.local_endpoint().port() << "/"
                            << (from.local_endpoint().address().is_v4() ?
                                    "ipv4" : "ipv6") << "] -> "
                            << "server=[" << to.remote_endpoint().address()
                            << ":" << to.remote_endpoint().port() << "/"
                            << (to.local_endpoint().address().is_v4() ?
                                    "ipv4" : "ipv6") << "] "
                            << "bytes=[" << bytes_transferred << "]";


                if (config_.client_delay_)
                    boost::this_thread::sleep_for(
                                boost::chrono::microseconds(
                                    config_.client_delay_));
            }

            if (config_.message_dump_ == hex)
            {
                hexdump(buffer_read.first.get(), bytes_transferred);
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
        if (!error_code)
        {
            LOG_ERROR() << "ec=[" << error_code << "] message=["
                        << error_code.message() << "]";
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
        const boost::system::error_code& error_code,
        size_t bytes_transferred,
        sp_buffer)
{
    LOG_TRACE() << "bytes sent: " << bytes_transferred;

    if (error_code)
    {
        LOG_ERROR() << "ec=[" << error_code << "] message=["
                    << error_code.message() << "]";
    }
}
