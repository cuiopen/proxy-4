//
//            Copyright (c) Marco Amorim 2015.
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

#define LOG_TRACE() BOOST_LOG_SEV(logger_, boost::log::trivial::trace)
#define LOG_DEBUG() BOOST_LOG_SEV(logger_, boost::log::trivial::debug)
#define LOG_INFO() BOOST_LOG_SEV(logger_, boost::log::trivial::info)
#define LOG_WARNING() BOOST_LOG_SEV(logger_, boost::log::trivial::warning)
#define LOG_ERROR() BOOST_LOG_SEV(logger_, boost::log::trivial::error)
#define LOG_FATAL() BOOST_LOG_SEV(logger_, boost::log::trivial::fatal)

void init_log_system(
        const std::string& settings_file,
        const std::string& severity_level);

typedef boost::log::sources::severity_channel_logger_mt< boost::log::trivial::severity_level, std::string > logger_type;
