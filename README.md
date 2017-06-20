## Proxy Manager

The proxy manager is a module that allows you to create TCP/IP proxies. These proxies can be helpful to debug and test network applications. The current implementation use the incredible [Boost.Asio](http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio.html) library and makes extensive use of asynchronous programming. This allows a better use of the system resources, which allows, for example, run several proxies with only one thread, each proxy managing multiple TCP sessions simultaneously.

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
I strongly recommend the use of a shadow build technique in order to keep the source directory clean:

```sh
$ mkdir build
$ cd build
$ cmake ${project_dir}
$ make
```

If everything goes well, you will end up with the module __proxy_manager__ on the root of the build directory. You can check the proxy manager version running:

```sh
$ cd ${build_dir}
$ ./proxy_manager --version
version 1.0.0
```

## Installation

It is possible to run the proxy manager without install it. But, if you wish, you can install it running:

```sh
$ make install
```
Be aware that you must have the right permissions in order to install the software.

## Usage
There are two operating modes:

The first mode uses a settings file which is used to initialize the proxy manager. This settings specifies the number of threads used by the thread pool, the logging setup, the list of proxies and so on.
You can get an example of this settings on the ${project_dir}/config/settings.xml.
Example:

```sh
$ proxy_manager --settings-file=${project_dir}/config/settings.xml
```

Also, you can use the short form:

```sh
$ proxy_manager -s${project_dir}/config/settings.xml
```

The second mode uses only the program arguments to configure and run only one proxy.
Example:

```sh
$ proxy_manager --name=http --sport=http-alt --shost=0.0.0.0 --dport=http --dhost=google.com --message-dump=ascii --log-level=debug
```

The example above will route all traffic from the alternative http port (8080) to the http port (80) on google.com. All messages will be dumped to the standard output in the ASCII format.

## API Reference

The API reference can be built with doxygen. If you have doxygen in your system just run:

```sh
$ make doc
```

 At the end of this process, the documentation will be available on:

${build-dir}/doc/html/index.html

 ## Docker

If you do not have time, or do not want to worry about the tech stuff, you can get a container ready to run  [here](https://hub.docker.com/r/mapamarco/pm_u16.04/). You can pull the latest image directly from the docker hub running the command:

```sh
$ docker pull mapamarco/pm_u16.04
```

This image was generated with the [${project_dir}/docker/Dockerfile](https://github.com/mapamarco/proxy/blob/master/docker/Dockerfile). If you want to build your own image based on this template, run:

```sh
$ cd ${project_dir}/docker
$ docker build -t pm_u16.04 .
```

After pulling a image or build a new one, you can use the proxy interactly:
```sh
# Fill the [DOCKER OPTIONS]
$ docker run -ti [DOCKER OPTIONS] pm_u16.04 proxy_manager [PROXY MANAGER OPTIONS]
```

Examples:
```sh
# Forwarding the traffic of the endpoint '0.0.0.0:2222/ipv4' to the endpoint '192.168.0.17:22/ipv4':
$ docker run -ti --rm -p2222:2222 pm_u16.04 proxy_manager --shost=0.0.0.0 --sport=2222 --dhost=192.168.0.17 --dport=22 --name=ssh

# Forwarding the traffic of the endpoint '0.0.0.0:2222/ipv4' to the endpoint '192.168.0.17:22/ipv4' with hexa dumping of messages:
$ docker run -ti --rm -p2222:2222 pm_u16.04 proxy_manager --shost=0.0.0.0 --sport=2222 --dhost=192.168.0.17 --dport=22 --name=ssh --message-dump=hex -ldebug

```

Using the interactly mode you can see the log messages and stop the proxy using CTRL+C directly on your terminal. But, sometimes we don't need to interact with the program, if this is your case, you can execute the proxy as a daemon using the option -d:

```sh
$ docker run -ti pm_u16.04 proxy_manager [OPTIONS]
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
 - Improve the connection drop (timeout)
 - Man page
 - Improve the documentation with UML diagrams
 - Improve the API reference
 - Improve this README
 - Add more examples
 - Bandwidth management

## License

This software is distributed under the Boost Software License, Version 1.0.
You can get a copy [here](https://github.com/mapamarco/proxy/blob/master/LICENSE_1_0.txt).
