//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include <iomanip>

#include <boost/make_shared.hpp>
#include <boost/asio.hpp>
#include <boost/foreach.hpp>
using namespace boost::asio;

#include "proxy.h"
using namespace net;

proxy::proxy(
        boost::asio::io_service& io_service,
        const proxy::config& config) :
       logger_(boost::log::keywords::channel = "net.proxy." + config.name_),
       io_service_(io_service),
       client_(io_service_),
       server_(io_service_),
       acceptor_(io_service_),
       resolver_(io_service_),
       from_(config.shost_, config.sport_),
       to_(config.dhost_, config.dport_),
       uniform_dist_(0, INT32_MAX),
       config_(config)
{
    LOG_INFO() << "ctor";
}

proxy::~proxy()
{
    LOG_INFO() << "dtor";
}

void proxy::start()
{
    LOG_INFO() << "starting proxy "
               << "src=[" << from_.host_name()
               << ":" << from_.service_name() << "] "
               << "dst=[" << to_.host_name()
               << ":" << to_.service_name() << "]";

    LOG_INFO() << "threads=[" << config_.thread_pool_size_ << "] "
               << "hexdump=[" << config_.hexdump_enabled_ << "] "
               << "buffer_size=[" << config_.buffer_size_ << "] ";

    LOG_INFO() << "client_delay=[" << config_.client_delay_ << "] "
               << "server_delay=[" << config_.server_delay_ << "]";

    resolver_.async_resolve(
                from_,
                boost::bind(
                    &proxy::handle_resolve,
                    this,
                    placeholders::error,
                    placeholders::iterator)
                );
}

void proxy::stop()
{
    BOOST_FOREACH(std::vector<session::ptr>::value_type& v, sessions_)
    {
        v->stop();
    }

    sessions_.clear();

    LOG_INFO() << "proxy stopped";
}

void proxy::handle_resolve(
        const boost::system::error_code& ec,
        boost::asio::ip::tcp::resolver::iterator it)
{
    if (!ec)
    {
        ip::tcp::resolver::iterator end;
        if (it != end)
        {
            ip::tcp::endpoint ep(*it);

            LOG_INFO() << "from endpoint=["
                       << ep.address() << ":" << ep.port() << "]";

            acceptor_.open(ep.protocol());
            acceptor_.set_option(socket_base::reuse_address(true));
            acceptor_.bind(ep);

            LOG_INFO() << "listening";
            acceptor_.listen();

            boost::system::error_code success;
            session::ptr null;
            handle_accept(success, null);
        }
    }
    else
    {
        LOG_ERROR() << "ec=[" << ec << "] message=[" << ec.message() << "]";
    }
}

void proxy::handle_accept(
        const boost::system::error_code& ec,
        session::ptr session_ptr)
{
    if (!ec)
    {
        std::ostringstream session_id;

        if (session_ptr)
        {
            LOG_INFO() << "connection accepted starting session=["
                       << session_ptr->get_id() << "]";

            session_ptr->start();

            sessions_.push_back(session_ptr);
        }

        session_id << std::hex << std::setfill('0') << std::setw(8)
                   << uniform_dist_(random_device_);

        session::ptr ptr =
                boost::make_shared<session>(
                    config_.name_,
                    boost::ref(io_service_),
                    session_id.str(),
                    to_.host_name(),
                    to_.service_name(),
                    config_.buffer_size_,
                    config_.hexdump_enabled_,
                    config_.client_delay_,
                    config_.server_delay_);

        acceptor_.async_accept(
                    ptr->get_socket(),
                    boost::bind(
                        &proxy::handle_accept,
                        this,
                        placeholders::error,
                        ptr));

    }
    else
    {
        LOG_ERROR() << "ec=[" << ec << "] message=[" << ec.message() << "]";
    }
}
