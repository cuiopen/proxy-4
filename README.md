## Proxy Manager

The Proxy Manager is a module that allows you to create TCP/IP proxies. These proxies can be helpful to debug and test network applications. The current implementation use the amazing [Boost.Asio](http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio.html) library and the asynchronous paradigm. This allows a better usage of the system resources, for example, you can run several proxies using only one thread, each proxy managing multiple TCP sessions simultaneously.

## Motivation
During the development of many projects involving networking applications, I realized that proxies are great tools that allow us debug and test the rightness of networking related code, at least at the application layer. With a proxy we can observe the network traffic without need a sniffer (which usually must be run in privileged mode). Besides that, we can simulate extreme scenarios in order to evaluate the robustness of these applications, test timeout scenarios, connection drop and another many cool things.
Over the years I developed a few proxies, mostly using the C language and the Socket API. But, after being presented to the extraordinary Boost.Asio library, I decided it was time to write a new version, simplifying the implementation and enjoying the wonders of the asynchronous paradigm. This project is the result of this work.

## Dependencies
If you want to build this project, you must have the following software installed in your system:

Mandatory:
 - A modern C++ compiler with support for C++11 features.
 - CMake >= 3.1
 - Boost.Libraries >= 1.55

Optional:
 - Doxygen (Needed if you want to generate the API reference)
 - Docker (Needed if you want use the dockerized version)

## Build
I strongly recommend the use of a shadow build in order to keep the source directory clean:

```sh
$ mkdir build
$ cd build
$ cmake ${project_dir}
$ make
```

If everything goes well, you will end with the module proxy_manager on the root of the build directory. You can check the Proxy Manager version running:

```sh
$ cd ${build_dir}
$ ./proxy_manager --version
version 1.0.0
```

## Usage
There are two operating modes:

The first mode uses a settings file which is used to initialize the Proxy Manager. This settings specifies the number of threads used by the thread pool, the logging setup, the list of proxies and so on.
You can get an example of this settings on the ${project_dir}/config/settings.xml.
Example:

```sh
$ proxy_manager --settings-file=${project_dir}/config/settings.xml
```

Also, you can use the short form:

```sh
$ proxy_manager -s${project_dir}/config/settings.xml
```

## Installation

It is possible to run the Proxy Manager without install the module. But, if you wish, you can install it running:

```sh
$ make install
```
Be aware that you must have the right permissions in order to install the software.

## API Reference

The API reference can be built with doxygen. If you have doxygen in your system just run:

```sh
$ make docs
```

 At the end of this process, the documentation will be available on the directory:

${build-dir}/doc

 ## Docker

If you do not have time, or do not want to worry about the tech stuff, you can get a container ready to run  at:

```sh
docker pull mapamarco/pm_u16.04
```

https://hub.docker.com/r/mapamarco/pm_u16.04/

This image was generated with the ${project_dir}/docker/Dockerfile

If you want to build your own image based on this template, run:

```sh
$ cd ${project_dir}/docker
$ docker build -t pm_u16.04 .
```

## Features
 - Multiples proxies per instance
 - IPv4 and IPv6 sockets
 - Asynchronous approach
 - Configurable logging system
 - Dump of messages (hexadecimal or ascii)
 - Configurable buffer sizes
 - Configurable message delays (client and server)
 - Thread pool

## TODO
 - UDP sockets
 - Add plugin support
 - Connection drop
 - Man page
 - Improve the documentation (UML diagrams)
 - Improve the API reference
 - Improve this README
 - Use Doxygen
 - Use Docker
 - Add more examples
 - Bandwidth management

## License

This software is distributed under the Boost Software License, Version 1.0.
You can get a copy [here](https://github.com/mapamarco/proxy/blob/master/LICENSE_1_0.txt).
