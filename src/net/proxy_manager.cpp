//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include "net/proxy_manager.h"
using namespace net;

proxy_manager::proxy_manager() :
    logger_(boost::log::keywords::channel = "net.proxy_manager"),
    signal_set_(io_service_)
{
    LOG_TRACE() << "ctor";

    signal_set_.add(SIGINT);

    signal_set_.async_wait(
                boost::bind(
                    &proxy_manager::handle_signal,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::signal_number));
}

proxy_manager::~proxy_manager()
{
    thread_group_.join_all();
    LOG_TRACE() << "dtor";
}

void proxy_manager::create_proxy(
        tcp_proxy::config& config)
{
    tcp_proxy::ptr proxy_ptr =
            boost::make_shared<tcp_proxy>(boost::ref(io_service_), config);

    proxies_[config.name_] = proxy_ptr;
    proxy_ptr->start();
}

void proxy_manager::start(
        const std::string& settings_file)
{
    LOG_INFO() << "starting";

    LOG_INFO() << "reading settings from file=[" << settings_file << "]";

    boost::property_tree::read_xml(settings_file, config_);

    BOOST_FOREACH(
                boost::property_tree::ptree::value_type& v,
                config_.get_child(CONFIG_ROOT + ".proxies"))
    {
        if (v.second.get("active", 0))
        {
            tcp_proxy::config config;

            config.name_ = v.second.get<std::string>("name");
            config.shost_ = v.second.get("shost", "localhost");
            config.dhost_ = v.second.get("dhost", "localhost");
            config.sport_ = v.second.get("sport", "http-alt");
            config.dport_ = v.second.get("dport", "http");
            config.client_delay_ = v.second.get("client-delay", 0);
            config.server_delay_ = v.second.get("server-delay", 0);
            config.buffer_size_ = v.second.get("buffer-size", 8192);
            config.message_dump_ =  v.second.get("message-dump", "none");

            create_proxy(config);
        }
    }

    const unsigned thread_pool_size =
            config_.get("proxy-settings.thread-pool.size",
                         boost::thread::hardware_concurrency());

    for (unsigned i = 0; i < thread_pool_size - 1; ++i)
    {
        thread_group_.create_thread(
                    boost::bind(&boost::asio::io_service::run,
                                &io_service_));
    }

    LOG_INFO() << "started";
    io_service_.run();
}

void proxy_manager::start(
        tcp_proxy::config proxy_config)
{
    LOG_INFO() << "starting";

    create_proxy(proxy_config);

    LOG_INFO() << "started";

    io_service_.run();
}

void proxy_manager::stop()
{
    LOG_INFO() << "stopping now";
    io_service_.stop();

    BOOST_FOREACH(proxy_map::value_type& v, proxies_)
    {
        v.second->stop();
    }

    proxies_.clear();
}

void proxy_manager::handle_signal(
        const boost::system::error_code& ec,
        int signal_number)
{
    if (!ec)
    {
        LOG_INFO() << "signal=[" << signal_number << "] received";

        if (signal_number == SIGINT)
        {
            stop();
        }
        else
        {
            signal_set_.async_wait(
                        boost::bind(
                            &proxy_manager::handle_signal,
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
