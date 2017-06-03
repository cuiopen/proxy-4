//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include "proxy_manager.h"
using namespace net;

proxy_manager::proxy_manager(
        const std::string& settings_file) :
    logger_(boost::log::keywords::channel = "net.proxy_manager"),
    signal_set_(io_service_)
{
    LOG_INFO() << "ctor";

    boost::property_tree::read_xml(settings_file, config_);

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
    LOG_INFO() << "dtor";
}

void proxy_manager::start()
{
    LOG_INFO() << "start";

    BOOST_FOREACH(
                boost::property_tree::ptree::value_type& v,
                config_.get_child("proxy-settings.proxies"))
    {
        proxy::config config;

        config.name_ = v.second.get<std::string>("name");
        config.shost_ = v.second.get("shost", "localhost");
        config.dhost_ = v.second.get("dhost", "localhost");
        config.sport_ = v.second.get("sport", "http-alt");
        config.dport_ = v.second.get("dport", "http");
        config.client_delay_ = v.second.get("client-delay", 0);
        config.server_delay_ = v.second.get("server-delay", 0);
        config.buffer_size_ = v.second.get("buffer-size", 8192);
        config.hexdump_enabled_ =  v.second.get("enable-hexdump", 0);

        proxy::ptr proxy_ptr =
                boost::make_shared<proxy>(boost::ref(io_service_), config);

        proxies_[config.name_] = proxy_ptr;

        proxy_ptr->start();
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

    io_service_.run();
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
            LOG_INFO() << "stopping now";
            io_service_.stop();

            BOOST_FOREACH(proxy_map::value_type& v, proxies_)
            {
                v.second->stop();
            }

            proxies_.clear();
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
