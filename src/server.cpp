#include <iostream>
#include <asio.hpp>

// using asio::ip;

namespace
{
    asio::io_context *io_context = nullptr;
    asio::ip::tcp::acceptor *acceptor = nullptr;
}


void start_accept();


void accept_handler(asio::ip::tcp::socket *socket, const asio::error_code& error)
{
    if (!error)
    {
        std::cout << "accepted connection!" << std::endl;
    }

    start_accept();
}

void start_accept()
{
    asio::ip::tcp::socket socket(*io_context);

    acceptor->async_accept(socket, 
        std::bind(accept_handler, &socket, asio::placeholders::error));
}


int main(int argc, char **argv)
{
    io_context = new asio::io_context();
    acceptor = new asio::ip::tcp::acceptor(*io_context, 
        asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 3777));

    start_accept();

    io_context->run();

    return 0;
}