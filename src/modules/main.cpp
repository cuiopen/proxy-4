//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>

#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "net/tcp_proxy.h"
#include "net/proxy_manager.h"
#include "core/log.h"

void add_options(
        boost::program_options::options_description& desc)
{
    namespace po = boost::program_options;

    desc.add_options()
            ("help,h",
             "this help message");

    desc.add_options()
            ("version,v",
             "show version info");

    desc.add_options()
            ("settings-file,s",
             po::value<std::string>(),
             "settings file");

    desc.add_options()
            ("message-dump,d",
             po::value<std::string>()->default_value("none"),
             "enable message dump of messages (ascii|hex|none)");

    desc.add_options()
            ("client-delay",
             po::value<size_t>()->default_value(0),
             "client delay (0 - disabled)");

    desc.add_options()
            ("server-delay",
             po::value<size_t>()->default_value(0),
             "server delay (0 - disabled)");

    desc.add_options()
            ("buffer-size,b",
             po::value<size_t>()->default_value(8192),
             "buffer size");

    desc.add_options()
            ("log-settings",
             po::value<std::string>()->default_value(""),
             "log settings file name");

    desc.add_options()
            ("log-level,l",
             po::value<std::string>()->default_value("info"),
             "log level (trace|debug|info|warning|error|fatal");

    desc.add_options()
            ("shost",
             po::value<std::string>()->default_value("localhost"),
             "source hostname");

    desc.add_options()
            ("sport",
             po::value<std::string>()->default_value("http-alt"),
             "source service name or port");

    desc.add_options()
            ("dhost",
             po::value<std::string>()->default_value("localhost"),
             "destination hostname");

     desc.add_options()
            ("dport",
             po::value<std::string>()->default_value("http"),
             "destination service name or port");
}

int main(int argc, char* argv[])
{
    const std::string MODULE_VERSION = "1.0.0";

    try
    {
        boost::program_options::variables_map vm;
        boost::program_options::options_description desc("allowed options");

        add_options(desc);

        boost::program_options::store(
                    boost::program_options::parse_command_line(
                        argc,
                        argv,
                        desc),
                    vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }

        if (vm.count("version"))
        {
            std::cout << "proxy version " << MODULE_VERSION << std::endl;
            return EXIT_SUCCESS;
        }

        if (vm.count("settings-file"))
        {
            boost::property_tree::ptree config;
            boost::property_tree::read_xml(
                        vm["settings-file"].as<std::string>(), config);

            const std::string LOGGING_ROOT = "proxy-settings.logging";

            core::logging::init(
                        config.get(LOGGING_ROOT + ".file-name", ""),
                        config.get(LOGGING_ROOT + ".severity", "info"));

            net::proxy_manager manager(
                        vm["settings-file"].as<std::string>());
            manager.start();
        }
        else
        {
            net::tcp_proxy::config config;

            core::logging::init(
                        vm["log-settings"].as<std::string>(),
                        vm["log-level"].as<std::string>());

            config.shost_ = vm["shost"].as<std::string>();
            config.dhost_ = vm["dhost"].as<std::string>();
            config.sport_ = vm["sport"].as<std::string>();
            config.dport_ = vm["dport"].as<std::string>();
            config.buffer_size_ = vm["buffer-size"].as<size_t>();
            config.message_dump_ = vm["message-dump"].as<std::string>();
            config.client_delay_ = vm["client-delay"].as<size_t>();
            config.server_delay_ = vm["server-delay"].as<size_t>();

            net::proxy_manager manager(config);
            manager.start();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "std::exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "unknown exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
