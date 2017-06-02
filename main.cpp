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

#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include "proxy.h"

int main(int argc, char* argv[])
{
    try
    {
        namespace po = boost::program_options;
        po::variables_map vm;
        po::options_description desc("allowed options");

        desc.add_options()
                ("help,h",
                 "this help message")
                ("disable-hexdump,d",
                 "disable hexdump of trace messages")
                ("buffer-size,b",
                 po::value<size_t>()->default_value(4096),
                 "buffer size")
                ("thread-pool-size,n",
                 po::value<size_t>()->default_value(
                     boost::thread::hardware_concurrency()),
                 "thread pool size")
                ("log-settings",
                 po::value<std::string>()->default_value(""),
                 "log settings file name")
                ("log-level,l",
                 po::value<std::string>()->default_value("info"),
                 "log level (trace|debug|info|warning|error|fatal")
                ("shost",
                 po::value<std::string>()->default_value("localhost"),
                 "source hostname")
                ("sport",
                 po::value<std::string>()->default_value("http-alt"),
                 "source service name or port")
                ("dhost",
                 po::value<std::string>()->default_value("localhost"),
                 "destination hostname")
                ("dport",
                 po::value<std::string>()->default_value("http"),
                 "destination service name or port");

        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }

        init_log_system(
                    vm["log-settings"].as<std::string>(),
                    vm["log-level"].as<std::string>());

        proxy service(
                    vm["shost"].as<std::string>(),
                    vm["sport"].as<std::string>(),
                    vm["dhost"].as<std::string>(),
                    vm["dport"].as<std::string>(),
                    vm["buffer-size"].as<size_t>(),
                    vm["thread-pool-size"].as<size_t>(),
                    !vm.count("disable-hexdump"));

        service.start();
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
