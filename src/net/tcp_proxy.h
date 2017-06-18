//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <string>
#include <map>
#include <cstdint>

#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "net/tcp_session.h"
#include "core/log.h"

///
/// @brief This namespace is used by all classes related to networking.
///
namespace net {

///
/// @brief This class is responsible for management of all tcp_session
/// instances.
///
class tcp_proxy :
        public boost::enable_shared_from_this<tcp_proxy>
{

public:

    ///
    /// @brief Defines a shared_ptr for itself.
    ///
    typedef boost::shared_ptr<tcp_proxy> ptr;

    ///
    /// @brief Defines a shared_ptr for the socket.
    ///
    typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

    ///
    /// @brief Defines a mapping between a session and its unique id.
    ///
    typedef std::map<std::string, tcp_session::ptr> session_map;

    ///
    /// @brief Defines the type of time_point used by the proxy.
    ///
    typedef boost::chrono::system_clock::time_point time_point;

    ///
    /// @brief This structure defines counters and time points for collecting
    /// statistical information.
    ///
    typedef struct info_
    {
        ///
        /// @brief Holds the data and time the proxy was started.
        ///
        time_point start_time_;

        ///
        /// @brief Holds the data and time the proxy was stopped.
        ///
        time_point stop_time_;

        ///
        /// @brief Holds the total of sessions processed.
        ///
        size_t total_sessions_;

        ///
        /// @brief Holds the sum of total bytes transmitted by all sessions.
        ///
        uint64_t total_tx_;

        ///
        /// @brief Holds the sum of total bytes received by all sessions.
        ///
        uint64_t total_rx_;

    } info;

    ///
    /// @brief This structures defines all configuration parameters required by
    /// the proxy.
    ///
    typedef struct config_
    {
        ///
        /// @brief Session name.
        ///
        std::string name_;

        ///
        /// @brief Source hostname or address.
        ///
        std::string shost_;

        ///
        /// @brief Source port or service name.
        ///
        std::string sport_;

        ///
        /// @brief Destination hostname or address.
        ///
        std::string dhost_;

        ///
        /// @brief Destination port or service name.
        ///
        std::string dport_;

        ///
        /// @brief This parameter specifies how long microseconds the messages
        /// from client will be delayed before being forwarded to the
        /// destination server.
        ///
        uint64_t client_delay_;

        ///
        /// @brief This parameter specifies how long microseconds the messages
        /// from server will be delayed before being forwarded to the
        /// destination client.
        ///
        uint64_t server_delay_;

        ///
        /// @brief This parameter specifies the size in bytes of the internal
        /// buffer used to forward messages.
        ///
        uint64_t buffer_size_;

        ///
        /// @brief This parameter specifies a time in microseconds to be used to
        /// terminate a inactive session by timeout.
        ///
        uint64_t timeout_;

        ///
        /// @brief Message dump type. Possible values are: "hex", "ascii" or
        /// "none".
        ///
        std::string message_dump_;

    } config;

    ///
    /// @brief Constructor.
    ///
    /// @param io_service Reference to io_service.
    /// @param proxy_config Proxy configuration.
    ///
    tcp_proxy(
            boost::asio::io_service& io_service,
            const config& proxy_config);

    ///
    /// @brief Destructor.
    ///
    virtual ~tcp_proxy();

    ///
    /// @brief Starts the proxy.
    ///
    virtual void start();

    ///
    /// @brief Stops all sessions and prints usage statistics.
    ///
    virtual void stop();

protected:

    ///
    /// @brief This handler is invoked by the session whenever it is finished.
    ///
    /// @param session_ptr The session that stopped.
    ///
    virtual void handle_session_stopped(
            tcp_session::ptr session_ptr);

    ///
    /// @brief This handler is invoked whenever the source hostname resolution
    /// has been completed.
    ///
    /// @param error_code The error code which indicates the result of the
    /// resolve operation.
    /// @param it The iterator to the endpoint list.
    ///
    virtual void handle_resolve(
            const boost::system::error_code& error_code,
            boost::asio::ip::tcp::resolver::iterator it);

    ///
    /// @brief This handler is invoked whenever there is an incoming connection.
    ///
    /// @param error_code The error code which indicates the result of the
    /// accept operation.
    /// @param session_ptr The session that will handle the new connection.
    ///
    virtual void handle_accept(
            const boost::system::error_code& error_code,
            tcp_session::ptr session_ptr);

    ///
    /// @brief Holds the logger responsible for logging events from objects of
    /// this class.
    ///
    core::logger_type logger_;

    ///
    /// @brief Holds the io_service reference used to process all asynchronous
    /// operations.
    ///
    boost::asio::io_service& io_service_;

    ///
    /// @brief Acceptor used to accept incoming connections.
    ///
    boost::asio::ip::tcp::acceptor acceptor_;

    ///
    /// @brief Resolver used to resolve hostnames.
    ///
    boost::asio::ip::tcp::resolver resolver_;

    ///
    /// @brief Query used to resolve the source hostname and service name.
    ///
    boost::asio::ip::tcp::resolver::query from_;

    ///
    /// @brief Query used to resolve the destination hostname and service name.
    ///
    boost::asio::ip::tcp::resolver::query to_;

    ///
    /// @brief This structure holds all active sessions indexed by their id.
    ///
    session_map sessions_;

    ///
    /// @brief Random device used to generate the sessions identifiers.
    ///
    boost::random::random_device random_device_;

    ///
    /// @brief Uniform distribution used to generate the sessions identifiers.
    ///
    boost::random::uniform_int_distribution<uint32_t> uniform_dist_;

    ///
    /// @brief Holds the configuration.
    ///
    config config_;

    ///
    /// @brief Holds statistical information.
    ///
    info info_;

};

} // namespace net
