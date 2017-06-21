//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <cstdint>

#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/chrono.hpp>
#include <boost/signals2.hpp>

#include "core/log.h"

///
/// @brief This namespace is used by all classes related to networking.
///
namespace net {

///
/// @brief This class is responsible for management one single TCP/IP session.
///
class tcp_session :
        public boost::enable_shared_from_this<tcp_session>
{

public:

    ///
    /// @brief Defines the message dump types.
    ///
    typedef enum message_dump_
    {
        none,       ///< Disables message dump.
        hex,        ///< Enables message dump in hexadecimal.
        ascii       ///< Enables message dump in ASCII.
    } message_dump;

    ///
    /// @brief Defines the possible session statuses.
    ///
    typedef enum status_
    {
        ready,      ///< The session is ready to start.
        running,    ///< The session already started.
        stopped     ///< The session is stopped.
    } status;

    ///
    /// @brief Defines a shared_ptr for itself.
    ///
    typedef boost::shared_ptr<tcp_session> ptr;

    ///
    /// @brief Defines a signal to handle session stop events.
    ///
    boost::signals2::signal<void(tcp_session::ptr)> signal_stopped_;

    ///
    /// @brief Defines a buffer type which combines the buffer and its size.
    ///
    typedef std::pair<boost::shared_ptr<uint8_t[]>, size_t> sp_buffer;

    ///
    /// @brief Defines the type of time_point used by the session.
    ///
    typedef boost::chrono::system_clock::time_point time_point;

    ///
    /// @brief This structure defines counters and time points for collecting
    /// statistical information.
    ///
    typedef struct info_
    {
        ///
        /// @brief Holds the current state of this session.
        ///
        status status_;

        ///
        /// @brief Holds the data and time the session was started.
        ///
        time_point start_time_;

        ///
        /// @brief Holds the data and time the session was stopped.
        ///
        time_point stop_time_;

        ///
        /// @brief Holds the sum of total bytes transmitted by this session.
        ///
        uint64_t total_tx_;

        ///
        /// @brief Holds the sum of total bytes received by this session.
        ///
        uint64_t total_rx_;

    } info;

    ///
    /// @brief This structures defines all configuration parameters required by
    /// the session.
    ///
    typedef struct config_
    {
        ///
        /// @brief Holds the session identified. A key composed by eight
        /// hexadecimal characters.
        ///
        std::string id_;

        ///
        /// @brief Holds the session name.
        ///
        std::string type_;

        ///
        /// @brief Holds the destination hostname or address.
        ///
        std::string host_;

        ///
        /// @brief Holds the destination por or service name.
        ///
        std::string port_;

        ///
        /// @brief Holds the buffer size used by this session.
        ///
        size_t buffer_size_;

        ///
        /// @brief Holds the period of time used to delay messages from client.
        /// It is expressed in microseconds.
        ///
        uint64_t client_delay_;

        ///
        /// @brief Holds the period of time used to delay messages from server.
        /// It is expressed in microseconds.
        ///
        uint64_t server_delay_;

        ///
        /// @brief Holds the timeout period used by the connection drop. It is
        /// expressed in microseconds.
        ///
        uint64_t timeout_;

        ///
        /// @brief Holds the message dump type.
        ///
        message_dump message_dump_;

    } config;

    ///
    /// @brief Constructor.
    ///
    /// @param io_service Reference to io_service.
    /// @param session_config Session configuration.
    ///
    tcp_session(
            boost::asio::io_service& io_service,
            const config& session_config);

    ///
    /// @brief Destructor.
    ///
    virtual ~tcp_session();

    ///
    /// @brief Gets the server socket associated with the session.
    ///
    /// @return The socket server.
    ///
    virtual boost::asio::ip::tcp::socket& get_socket();

    ///
    /// @brief Starts the session.
    ///
    virtual void start();

    ///
    /// @brief Stops all connections and asynchronous operations. After that, a
    /// stopped signal will be emited to his proxy owner.
    ///
    virtual void stop();

    ///
    /// @brief Gets the session identifier.
    ///
    /// @return The session identifier.
    ///
    virtual const std::string& get_id();

    ///
    /// @brief Gets statistical information.
    ///
    /// @return Statistical information like the total bytes received and
    /// transmitted, session start time, etc.
    ///
    virtual const info& get_info();

protected:

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
    /// @brief Handles a timeout event.
    ///
    /// @param error_code The error code which indicates the result of the
    /// async_wait operation.
    ///
    virtual void handle_timeout(
            const boost::system::error_code& error_code);

    ///
    /// @brief Handles a connection event.
    ///
    /// @param error_code The error code which indicates the result of the
    /// connect operation.
    ///
    virtual void handle_connect(
            const boost::system::error_code& error_code);

    ///
    /// @brief Handles a read event.
    ///
    /// @param error_code The error code which indicates the result of the
    /// read operation.
    /// @param bytes_transferred Total amount of bytes received.
    /// @param buffer Source buffer that contains the bytes received.
    /// @param from Source socket.
    /// @param to Destination socket.
    /// @param server_flag Flag indicating whether it is a message from the
    /// server.
    ///
    virtual void handle_read(
            const boost::system::error_code& error_code,
            size_t bytes_transferred,
            sp_buffer buffer,
            boost::asio::ip::tcp::socket& from,
            boost::asio::ip::tcp::socket& to,
            bool server_flag);

    ///
    /// @brief Handles a send event.
    ///
    /// @param error_code The error code which indicates the result of the
    /// connect operation.
    /// @param bytes_transferred Total amount of bytes transmitted.
    /// @param buffer Source buffer.
    ///
    virtual void handle_send(
            const boost::system::error_code& error_code,
            size_t bytes_transferred,
            sp_buffer buffer);

    ///
    /// @brief Sets a session timeout. This is useful to drops inactive
    /// connections.
    ///
    /// @param timeout Timeout expressed in microseconds.
    ///
    virtual void set_timeout(
            uint64_t timeout);

    ///
    /// @brief Prints the hexadecimal representation of a buffer.
    ///
    /// @param buffer Buffer that will be printed.
    /// @param size Buffer size.
    ///
    void hexdump(
            const uint8_t* buffer,
            size_t size);

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
    /// @brief Holds the socket from the client side.
    ///
    boost::asio::ip::tcp::socket client_;

    ///
    /// @brief Holds the socket from the server side.
    ///
    boost::asio::ip::tcp::socket server_;

    ///
    /// @brief Resolver used to resolve the destination hostname.
    ///
    boost::asio::ip::tcp::resolver resolver_;

    ///
    /// @brief Query used to resolve the destination service name.
    ///
    boost::asio::ip::tcp::resolver::query to_;

    ///
    /// @brief Timer used to handle connection drop by timeout.
    ///
    boost::asio::deadline_timer timeout_timer_;

    ///
    /// @brief Timer used to handle server delays.
    ///
    boost::asio::deadline_timer server_timer_;

    ///
    /// @brief Timer used to handle client delays.
    ///
    boost::asio::deadline_timer client_timer_;

    ///
    /// @brief Holds the configuration.
    ///
    info info_;

    ///
    /// @brief Holds statistical information.
    ///
    config config_;

    ///
    /// @brief Mutex used to synchronize access to this class.
    ///
    boost::mutex mutex_;

};

} // namespace net
