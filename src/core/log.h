//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <string>

#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/channel_logger.hpp>
#include <boost/log/attributes.hpp>

///
/// @brief These macros are helpful for easy printing of log messages.
///
#define LOG_TRACE() BOOST_LOG_SEV(logger_, boost::log::trivial::trace)
#define LOG_DEBUG() BOOST_LOG_SEV(logger_, boost::log::trivial::debug)
#define LOG_INFO() BOOST_LOG_SEV(logger_, boost::log::trivial::info)
#define LOG_WARNING() BOOST_LOG_SEV(logger_, boost::log::trivial::warning)
#define LOG_ERROR() BOOST_LOG_SEV(logger_, boost::log::trivial::error)
#define LOG_FATAL() BOOST_LOG_SEV(logger_, boost::log::trivial::fatal)

///
/// @brief This namespace is used by all core classes.
///
namespace core {

///
/// @brief This class is responsible for logging.
///
class logging
{
public:

    ///
    /// @brief Initialises the logging system.
    ///
    /// @param settings_file Full path of the settings file that contains the
    /// log configuration.
    /// @param severity_level Specifies the current severity level used by the
    /// logging system. Possible values are: "trace", "debug", "info",
    /// "warning", "error" or "fatal".
    ///
    static void init(
            const std::string& settings_file,
            const std::string& severity_level);
};

///
/// @brief Defines the logger type that is used to generate log messages.
///
typedef boost::log::sources::severity_channel_logger_mt<
    boost::log::trivial::severity_level, std::string> logger_type;

} // namespace core
