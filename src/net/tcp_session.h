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
    /// @brief Message dump types.
    ///
    typedef enum message_dump_
    {
        none,
        hex,
        ascii
    } message_dump;

    typedef enum status_
    {
        ready,
        running,
        stopped
    } status;

    ///
    /// @brief Defines a shared_ptr for itself.
    ///
    typedef boost::shared_ptr<tcp_session> ptr;

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
        status status_;
        time_point start_time_;
        time_point stop_time_;
        uint64_t total_tx_;
        uint64_t total_rx_;
    } info;

    ///
    /// @brief This structures defines all configuration parameters required by
    /// the session.
    ///
    typedef struct config_
    {
        std::string id_;
        std::string type_;
        std::string host_;
        std::string port_;
        size_t buffer_size_;
        uint64_t client_delay_;
        uint64_t server_delay_;
        uint64_t timeout_;
        message_dump message_dump_;
    } config;

    tcp_session(
            boost::asio::io_service& io_service,
            const config& session_config);

    virtual ~tcp_session();

    boost::asio::ip::tcp::socket& get_socket();

    void start();

    void stop();

    const std::string& get_id();

    const info& get_info();

    boost::signals2::signal<void(tcp_session::ptr)> signal_stopped_;

protected:

    void handle_resolve(
            const boost::system::error_code& ec,
            boost::asio::ip::tcp::resolver::iterator it);

    void handle_timeout(
            const boost::system::error_code& ec);

    void handle_connect(
            const boost::system::error_code& ec);

    void handle_read(
            const boost::system::error_code& ec,
            size_t bytes_tranferred,
            sp_buffer buffer,
            boost::asio::ip::tcp::socket& from,
            boost::asio::ip::tcp::socket& to,
            bool server_flag);

    void handle_send(
            const boost::system::error_code& ec,
            size_t bytes_tranferred,
            sp_buffer buffer);

    void set_timeout(
            uint64_t timeout);

    void hexdump(
            size_t bytes_tranferred,
            sp_buffer buffer);

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

    boost::asio::ip::tcp::socket client_;

    boost::asio::ip::tcp::socket server_;

    boost::asio::ip::tcp::resolver resolver_;

    boost::asio::ip::tcp::resolver::query to_;

    boost::asio::deadline_timer timeout_timer_;

    boost::asio::deadline_timer server_timer_;

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
