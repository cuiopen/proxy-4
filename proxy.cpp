//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include <iomanip>

#include <boost/make_shared.hpp>

#include "proxy.h"

using namespace boost::asio;

proxy::proxy(
        const std::string& shost,
        const std::string& sport,
        const std::string& dhost,
        const std::string& dport,
        size_t buffer_size,
        size_t thread_pool_size,
        bool enable_hexdump) :
    logger_(boost::log::keywords::channel = "net.proxy"),
    client_(io_service_),
    server_(io_service_),
    acceptor_(io_service_),
    resolver_(io_service_),
    from_(shost, sport),
    to_(dhost, dport),
    uniform_dist_(0, INT32_MAX),
    buffer_size_(buffer_size),
    signal_set_(io_service_),
    thread_pool_size_(thread_pool_size),
    hexdump_enabled_(enable_hexdump)
{
    signal_set_.add(SIGINT);
    signal_set_.async_wait(
                boost::bind(
                    &proxy::handle_signal,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::signal_number));
}

proxy::~proxy()
{
    thread_group_.join_all();
    LOG_INFO() << "proxy destroyed";
}

void proxy::start()
{
    LOG_INFO() << "starting proxy "
               << "src=[" << from_.host_name()
               << ":" << from_.service_name() << "] "
               << "dst=[" << to_.host_name()
               << ":" << to_.service_name() << "]";

    LOG_INFO() << "threads=[" << thread_pool_size_ << "] "
               << "hexdump=[" << hexdump_enabled_ << "] "
               << "buffer_size=[" << buffer_size_ << "]";

    resolver_.async_resolve(
                from_,
                boost::bind(
                    &proxy::handle_resolve,
                    this,
                    placeholders::error,
                    placeholders::iterator)
                );

    for (size_t i = 0; i < thread_pool_size_ - 1; ++i)
    {
        thread_group_.create_thread(
                    boost::bind(&boost::asio::io_service::run,
                                &io_service_));
    }

    io_service_.run();
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
            LOG_INFO() << "connection accepted";

            session_ptr->start();

            sessions_.push_back(session_ptr);
        }

        session_id << std::hex << std::setfill('0') << std::setw(8)
                   << uniform_dist_(random_device_);

        session::ptr ptr =
                boost::make_shared<session>(
                    boost::ref(io_service_),
                    session_id.str(),
                    to_.host_name(),
                    to_.service_name(),
                    buffer_size_,
                    hexdump_enabled_);

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

void proxy::handle_signal(
        const boost::system::error_code& ec,
        int signal_number)
{
    if (!ec)
    {
        LOG_INFO() << "signal=[" << signal_number << "] received";
        if (signal_number == SIGINT)
        {
            LOG_INFO() << "stopping now";
            io_service_.stop();
        }
        else
        {
            signal_set_.async_wait(
                        boost::bind(
                            &proxy::handle_signal,
                            this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::signal_number));
        }
    }
    else
    {
        LOG_ERROR() << "ec=[" << ec << "] message=[" << ec.message() << "]";
    }
}
