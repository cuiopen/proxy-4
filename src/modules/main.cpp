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

#include "proxy.h"
#include "proxy_manager.h"
#include "log.h"

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
            ("disable-hexdump,d",
             "disable hexdump of trace messages");

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
            ("thread-pool-size,n",
             po::value<size_t>()->default_value(
                 boost::thread::hardware_concurrency()),
             "thread pool size");

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

void load_settings(
        const std::string& file)
{

}

int main(int argc, char* argv[])
{
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
            std::cout << "proxy version 1.0.0" << std::endl;
            return EXIT_SUCCESS;
        }

        core::logging::init(
                    vm["log-settings"].as<std::string>(),
                    vm["log-level"].as<std::string>());

        if (vm.count("settings-file"))
        {
            net::proxy_manager manager(
                        vm["settings-file"].as<std::string>());

            manager.start();
        }
        else
        {
            net::proxy::config proxy_config;
            boost::asio::io_service io_service;

            proxy_config.shost_ = vm["shost"].as<std::string>();
            proxy_config.dhost_ = vm["dhost"].as<std::string>();
            proxy_config.sport_ = vm["sport"].as<std::string>();
            proxy_config.dport_ = vm["dport"].as<std::string>();
            proxy_config.buffer_size_ = vm["buffer-size"].as<size_t>();
            proxy_config.hexdump_enabled_ = !vm.count("disable-hexdump");
            proxy_config.client_delay_ = vm["client-delay"].as<size_t>();
            proxy_config.server_delay_ = vm["server-delay"].as<size_t>();
            //vm["thread-pool-size"].as<size_t>();

            net::proxy service(boost::ref(io_service), proxy_config);

            service.start();
            io_service.run();
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
