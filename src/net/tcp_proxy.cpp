//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
using namespace boost::asio;

#include "net/tcp_proxy.h"
using namespace net;

tcp_proxy::tcp_proxy(
        boost::asio::io_service& io_service,
        const tcp_proxy::config& config) :
       logger_(boost::log::keywords::channel = "net.tcp_proxy." + config.name_),
       io_service_(io_service),
       acceptor_(io_service_),
       resolver_(io_service_),
       from_(config.shost_, config.sport_),
       to_(config.dhost_, config.dport_),
       uniform_dist_(0, UINT32_MAX),
       config_(config)
{
    LOG_TRACE() << "ctor";
    memset(&info_, 0, sizeof(info_));
}

tcp_proxy::~tcp_proxy()
{
    LOG_TRACE() << "dtor";
}

void tcp_proxy::start()
{
    info_.start_time_ = boost::chrono::system_clock::now();

    LOG_INFO() << "starting source=[" << from_.host_name() << ":"
               << from_.service_name() << "] "
               << "destination=[" << to_.host_name() << ":"
               << to_.service_name() << "]";

    LOG_INFO() << "message-dump=[" << config_.message_dump_ << "] "
               << "buffer-size=[" << config_.buffer_size_ << "] "
               << "timeout=[" << config_.timeout_ << "]";

    LOG_INFO() << "client-delay=[" << config_.client_delay_ << "] "
               << "server-delay=[" << config_.server_delay_ << "]";

    resolver_.async_resolve(
                from_,
                boost::bind(
                    &tcp_proxy::handle_resolve,
                    this,
                    placeholders::error,
                    placeholders::iterator)
                );
}

void tcp_proxy::stop()
{
    BOOST_FOREACH(session_map::value_type& v, sessions_)
    {
        v.second->stop();
    }

    info_.stop_time_ = boost::chrono::system_clock::now();

    LOG_INFO() << "stats "
               << "sessions=[" << info_.total_sessions_ << "] "
               << "tx=[" << info_.total_tx_ << "] "
               << "rx=[" << info_.total_rx_ << "] "
               << "elapsed=[" << boost::chrono::duration_cast<
                  boost::chrono::milliseconds>(
                      info_.stop_time_ - info_.start_time_)
               << "]";
    LOG_DEBUG() << "stopped";
}

void tcp_proxy::handle_session_stopped(
        tcp_session::ptr session_ptr)
{
    boost::lock_guard<boost::mutex> lock(mutex_);

    LOG_INFO() << "removing session=[" << session_ptr->get_id() << "]";

    info_.total_rx_ += session_ptr->get_info().total_rx_;
    info_.total_tx_ += session_ptr->get_info().total_tx_;
    ++info_.total_sessions_;

    sessions_.erase(session_ptr->get_id());
}

void tcp_proxy::handle_resolve(
        const boost::system::error_code& error_code,
        boost::asio::ip::tcp::resolver::iterator it)
{
    if (!error_code)
    {
        ip::tcp::resolver::iterator end;

        if (it != end)
        {
            ip::tcp::endpoint ep(*it);

            LOG_INFO() << "binding endpoint=["
                       << ep.address() << ":" << ep.port() << "/"
                       << (ep.address().is_v4() ? "ipv4" : "ipv6") << "]";

            acceptor_.open(ep.protocol());
            acceptor_.set_option(socket_base::reuse_address(true));
            acceptor_.bind(ep);

            LOG_INFO() << "listening";

            acceptor_.listen();

            boost::system::error_code success;
            tcp_session::ptr null;
            handle_accept(success, null);
        }
    }
    else
    {
        LOG_ERROR() << "ec=[" << error_code << "] message=["
                    << error_code.message() << "]";
    }
}

void tcp_proxy::handle_accept(
        const boost::system::error_code& error_code,
        tcp_session::ptr session_ptr)
{
    boost::lock_guard<boost::mutex> lock(mutex_);

    if (!error_code)
    {
        std::ostringstream session_id;

        if (session_ptr)
        {
            LOG_INFO() << "connection accepted - session=["
                       << session_ptr->get_id() << "]";

            session_ptr->start();

            sessions_[session_ptr->get_id()] = session_ptr;
        }

        session_id << std::hex << std::setfill('0') << std::setw(8)
                   << uniform_dist_(random_device_);

        tcp_session::config session_config;

        session_config.id_ = session_id.str();
        session_config.type_ = config_.name_;
        session_config.buffer_size_ = config_.buffer_size_;
        session_config.host_ = to_.host_name();
        session_config.port_ = to_.service_name();
        session_config.client_delay_ = config_.client_delay_;
        session_config.server_delay_ = config_.server_delay_;
        session_config.timeout_ = config_.timeout_;

        if (config_.message_dump_ == "hex")
        {
            session_config.message_dump_ = tcp_session::hex;
        }
        else if (config_.message_dump_ == "ascii")
        {
            session_config.message_dump_ = tcp_session::ascii;
        }
        else
        {
            session_config.message_dump_ = tcp_session::none;
        }

        tcp_session::ptr ptr =
                boost::make_shared<tcp_session>(
                    boost::ref(io_service_),
                    session_config);

        ptr->signal_stopped_.connect(
                    boost::bind(
                        &tcp_proxy::handle_session_stopped,
                        this,
                        _1));

        acceptor_.async_accept(
                    ptr->get_socket(),
                    boost::bind(
                        &tcp_proxy::handle_accept,
                        this,
                        placeholders::error,
                        ptr));

    }
    else
    {
        LOG_ERROR() << "ec=[" << error_code << "] message=[" << error_code.message() << "]";
    }
}
