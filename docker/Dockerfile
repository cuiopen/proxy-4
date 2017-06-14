#
#            Copyright (c) Marco Amorim 2017.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#

# Select the base image
FROM ubuntu:16.04

MAINTAINER Marco Amorim <mapamarco@gmail.com>

# Update and install packages
RUN apt -y update
RUN apt -y install g++ cmake make libboost-all-dev doxygen git graphviz

# Create users and groups
RUN groupadd -g 1000 dev
RUN useradd -m -u 1000 -g dev -s /bin/bash dev

# Clone and build the module
RUN su - dev
RUN git clone https://github.com/mapamarco/proxy.git /home/dev/proxy && \
    cd /home/dev && \
    mkdir build && \
    cd build && \
    cmake ../proxy && \
    make install

# Set the default user, workdir and so on
USER dev
WORKDIR /home/dev
CMD /usr/local/bin/proxy_manager
