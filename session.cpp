//
//            Copyright (c) Marco Amorim 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "session.h"

#include <iomanip>
#include <sstream>

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>

session::session(
        boost::asio::io_service& io_service,
        const std::string& session_id,
        const std::string& host,
        const std::string& port,
        size_t buffer_size,
        bool enable_hexdump) :
    logger_(boost::log::keywords::channel =
        std::string("net.session." + session_id)),
    io_service_(io_service),
    client_(io_service),
    server_(io_service),
    resolver_(io_service),
    to_(host, port),
    buffer_size_(buffer_size),
    hexdump_enabled_(enable_hexdump)
{
    LOG_DEBUG() << "created";
}

session::~session()
{
    LOG_DEBUG() << "destroyed";
}

boost::asio::ip::tcp::socket& session::get_socket()
{
    return server_;
}

void session::start()
{
    LOG_DEBUG() << "session started";

    resolver_.async_resolve(
                to_,
                boost::bind(
                    &session::handle_resolve,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::iterator)
                );
}

void session::handle_resolve(
        const boost::system::error_code& ec,
        boost::asio::ip::tcp::resolver::iterator it)
{
    if (!ec)
    {
        boost::asio::ip::tcp::resolver::iterator end;
        if (it != end)
        {
            boost::asio::ip::tcp::endpoint ep(*it);

            LOG_DEBUG() << "to endpoint=["
                        << ep.address() << ":" << ep.port() << "]";

            client_.async_connect(
                        ep,
                        boost::bind(
                            &session::handle_connect,
                            this,
                            boost::asio::placeholders::error));
        }
    }
    else
    {
        LOG_ERROR() << "ec=[" << ec << "] message=[" << ec.message() << "]";
    }

}

void session::handle_connect(
        const boost::system::error_code& ec)
{
    if (!ec)
    {
        LOG_DEBUG() << "connected";

        try
        {
            sp_buffer buffer =
                    std::make_pair(
                        boost::make_shared<uint8_t[]>(buffer_size_),
                        buffer_size_);

            client_.async_read_some(
                        boost::asio::buffer(
                            buffer.first.get(), buffer.second),
                        boost::bind(
                            &session::handle_read, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred,
                            buffer,
                            boost::ref(client_),
                            boost::ref(server_)));

            buffer =
                    std::make_pair(
                        boost::make_shared<uint8_t[]>(buffer_size_),
                        buffer_size_);

            server_.async_read_some(
                        boost::asio::buffer(
                            buffer.first.get(), buffer.second),
                        boost::bind(
                            &session::handle_read, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred,
                            buffer,
                            boost::ref(server_),
                            boost::ref(client_)));
        }
        catch (std::exception& e)
        {
            LOG_ERROR() << "std::exception what=[" << e.what() << "]";
        }
    }
    else
    {
        LOG_ERROR() << " ec=[" << ec << "] message=[" << ec.message() << "]";
    }
}

void session::hexdump(
        size_t bytes_tranferred,
        sp_buffer buffer)
{
    size_t i = 0;
    uint32_t address = 0;
    std::ostringstream hex_out, ascii_out;

    hex_out << std::setfill('0') << std::setw(8) << address << "    ";

    while (i < bytes_tranferred)
    {
        hex_out << std::hex << std::setw(2)
                << static_cast<uint16_t>(buffer.first[i]) << ' ';

        ascii_out << static_cast<char>(
                         std::isgraph(buffer.first[i]) ? buffer.first[i] : '.');

        ++i;

        if (!(i % 16))
        {
            LOG_TRACE() << hex_out.str() << "   " << ascii_out.str();

            ascii_out.str("");
            hex_out.str("");

            address += 16;
            hex_out << std::setw(8) << address << "    ";
        }
    }

    if (i % 16)
    {
        while (i++ % 16)
        {
            hex_out << "   ";
        }

        LOG_TRACE() << hex_out.str() << "   " << ascii_out.str();
    }
}

void session::handle_read(
        const boost::system::error_code& ec,
        size_t bytes_tranferred,
        sp_buffer buffer_read,
        boost::asio::ip::tcp::socket& from,
        boost::asio::ip::tcp::socket& to)
{
    if (!ec && bytes_tranferred)
    {
        try
        {
            to.async_send(
                        boost::asio::buffer(
                            buffer_read.first.get(),
                            bytes_tranferred),
                        boost::bind(
                            &session::handle_send,
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred,
                            buffer_read));

            if (client_.local_endpoint() == to.local_endpoint())
            {
                LOG_TRACE() << "client=[" << from.local_endpoint().address()
                            << ":" << from.local_endpoint().port() << "] -> "
                            << "server=[" << to.remote_endpoint().address()
                            << ":" << to.remote_endpoint().port() << "] "
                            << "bytes=[" << bytes_tranferred << "]";
            }
            else
            {
                LOG_TRACE() << "server=[" << from.local_endpoint().address()
                            << ":" << from.local_endpoint().port() << "] -> "
                            << "client=[" << to.remote_endpoint().address()
                            << ":" << to.remote_endpoint().port() << "] "
                            << "bytes=[" << bytes_tranferred << "]";
            }

            if (hexdump_enabled_)
                hexdump(bytes_tranferred, buffer_read);

            sp_buffer buffer =
                    std::make_pair(
                        boost::make_shared<uint8_t[]>(buffer_size_),
                        buffer_size_);

            from.async_read_some(
                        boost::asio::buffer(
                            buffer.first.get(), buffer.second),
                        boost::bind(
                            &session::handle_read, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred,
                            buffer,
                            boost::ref(from),
                            boost::ref(to)));
        }
        catch (std::exception& e)
        {
            LOG_ERROR() << "std::exception what=[" << e.what() << "]";
        }
    }
    else
    {
        if (!ec)
        {
            LOG_ERROR() << "ec=[" << ec << "] message=[" << ec.message() << "]";
        }
        else
        {
            LOG_DEBUG() << "connection closed";
        }

        server_.close();
    }

}

void session::handle_send(
        const boost::system::error_code& ec,
        size_t bytes_tranferred,
        sp_buffer)
{
    LOG_TRACE() << "bytes sent: " << bytes_tranferred;

    if (ec)
    {
        LOG_ERROR() << "ec=[" << ec << "] message=[" << ec.message() << "]";
    }
}
