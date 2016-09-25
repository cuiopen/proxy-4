//
//            Copyright (c) Marco Amorim 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include <string>
#include <sstream>

#include <boost/program_options.hpp>

#include "proxy.h"

int main(int argc, char** argv)
{
    try
    {
        namespace po = boost::program_options;
        po::variables_map vm;
        po::options_description desc("Allowed options");
        desc.add_options()
                ("help,h", "this help message")
                ("log-settings", po::value<std::string>()->default_value(""), "log settings file name")
                ("log-level,l", po::value<std::string>()->default_value("info"), "log level (trace|debug|info|warning|error|fatal")
                ("shost", po::value<std::string>()->default_value("localhost"), "source hostname")
                ("sport", po::value<std::string>()->default_value("http-alt"), "source service name or port")
                ("dhost", po::value<std::string>()->default_value("localhost"), "destination hostname")
                ("dport", po::value<std::string>()->default_value("http"), "destination service name or port");

        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return 0;
        }

        init_log_system(
                    vm["log-settings"].as<std::string>(),
                    vm["log-level"].as<std::string>());

        boost::asio::io_service io_service;
        proxy service(
                    io_service,
                    vm["shost"].as<std::string>(),
                    vm["sport"].as<std::string>(),
                    vm["dhost"].as<std::string>(),
                    vm["dport"].as<std::string>());

        service.start();
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "std::exception: " << e.what() << std::endl;
    }

    return 0;
}
