//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <string>
#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#include "net/tcp_proxy.h"
#include "core/log.h"

namespace net {

class proxy_manager
{
public:

    typedef std::map<std::string, tcp_proxy::ptr> proxy_map;

    proxy_manager();

    virtual ~proxy_manager();

    virtual void start(
            const std::string& settings_file);

    virtual void start(
            tcp_proxy::config proxy_config);

    virtual void stop();

protected:

    const std::string CONFIG_ROOT = "proxy-settings";

    void create_proxy(
            tcp_proxy::config& config);

    void handle_signal(
            const boost::system::error_code& ec,
            int signal_number);

    core::logger_type logger_;

    boost::property_tree::ptree config_;

    proxy_map proxies_;

    boost::thread_group thread_group_;

    boost::asio::io_service io_service_;

    boost::asio::signal_set signal_set_;

};

} // namespace net
