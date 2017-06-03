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
#include <boost/thread/thread_pool.hpp>

#include "proxy.h"
#include "log.h"

namespace net {

class proxy_manager
{
public:

    typedef std::map<std::string, proxy::ptr> proxy_map;

    proxy_manager(
            const std::string& settings_file);

    virtual ~proxy_manager();

    virtual void start();

protected:

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
