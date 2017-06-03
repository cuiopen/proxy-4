//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <map>
#include <fstream>
#include <sstream>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/from_stream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/trivial.hpp>
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace trivial = boost::log::trivial;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;

#include "log.h"
using namespace core;

BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", trivial::severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(channel, "Channel", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(thread_id, "ThreadID",
                            attrs::current_thread_id::value_type)

void logging::init(
        const std::string& settings_file,
        const std::string& severity_level)
{
	boost::log::register_simple_formatter_factory<
			trivial::severity_level, char>("Severity");
	boost::log::register_simple_filter_factory<
			trivial::severity_level, char>("Severity");
	boost::log::add_common_attributes();

    if (!settings_file.empty())
    {
        std::ifstream settings(settings_file.c_str());
        if (!settings.is_open())
        {
            std::ostringstream errm;
            errm << "could not open " << settings_file << " file";
            throw std::invalid_argument(errm.str());
        }

        boost::log::init_from_stream(settings);
    }
    else
    {
        std::map< std::string, trivial::severity_level > severity_map;
        severity_map["trace"] = trivial::trace;
        severity_map["debug"] = trivial::debug;
        severity_map["info"] = trivial::info;
        severity_map["warning"] = trivial::warning;
        severity_map["error"] = trivial::error;
        severity_map["fatal"] = trivial::fatal;

        boost::log::add_console_log(
            std::clog,
            keywords::format =
            (
                expr::stream
                    << expr::format_date_time(timestamp, "%Y-%m-%d %H:%M:%S.%f")
                    << ": {" << thread_id << "} "
                    << "<" << severity
                    << "> [" << channel << "] "
                    << expr::smessage
            )
        );

        boost::log::core::get()->set_filter
        (
            trivial::severity >= severity_map[severity_level]
        );
    }
}
