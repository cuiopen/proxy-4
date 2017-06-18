//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <string>
#include <map>

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/property_tree/ptree.hpp>

#include "net/tcp_proxy.h"
#include "core/log.h"

///
/// @brief This namespace is used by all classes related to networking.
///
namespace net {

///
/// @brief This class is responsible for management of all proxies instances.
///
class proxy_manager
{
public:

    ///
    /// @brief Defines a mapping between a proxy and its name.
    ///
    typedef std::map<std::string, tcp_proxy::ptr> proxy_map;

    ///
    /// @brief Constructor. Adds and initiates a signal handler for system
    /// signals.
    ///
    proxy_manager();

    ///
    /// @brief Destructor. Joins all threads.
    ///
    virtual ~proxy_manager();

    ///
    /// @brief Starts one or more proxies based on a settings_file.
    ///
    /// @param settings_file The full path of the settings file that will be
    /// used.
    ///
    virtual void start(
            const std::string& settings_file);

    ///
    /// @brief Starts one proxy based on a proxy configuration.
    ///
    /// @param proxy_config The proxy configuration that will be used.
    ///
    virtual void start(
            const tcp_proxy::config& proxy_config);

    ///
    /// @brief Stops the io_service and all proxy instances.
    ///
    virtual void stop();

protected:

    ///
    /// @brief This constant defines the name of the root node used by the
    /// settings file.
    ///
    const std::string CONFIG_ROOT = "proxy-settings";

    ///
    /// @brief Creates a new proxy based on a configuration.
    ///
    /// @param config Proxy configuration.
    ///
    virtual void create_proxy(
            const tcp_proxy::config& config);

    ///
    /// @brief Handles a signal.
    ///
    /// @param erro_code Error code indicating the result of the operation.
    /// @param signal_number Number of the signal.
    ///
    virtual void handle_signal(
            const boost::system::error_code& erro_code,
            int signal_number);

    ///
    /// @brief Holds the logger responsible for logging events from objects of
    /// this class.
    ///
    core::logger_type logger_;

    ///
    /// @brief Holds the configuration.
    ///
    boost::property_tree::ptree config_;

    ///
    /// @brief This structure holds all active proxies.
    ///
    proxy_map proxies_;

    ///
    /// @brief Holds the additional threads used by the io_service.
    ///
    boost::thread_group thread_group_;

    ///
    /// @brief Holds the io_service used to process all asynchronous operations.
    ///
    boost::asio::io_service io_service_;

    ///
    /// @brief Holds the set of signals that are mapped from this class.
    ///
    boost::asio::signal_set signal_set_;

    ///
    /// @brief Mutex used to synchronize access to this class.
    ///
    boost::mutex mutex_;
};

} // namespace net
