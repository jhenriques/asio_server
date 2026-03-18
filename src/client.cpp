#include <iostream>
#include <asio.hpp>

int main(int argc, char **argv)
{
    asio::io_context io_context;

    asio::ip::tcp::resolver resolver(io_context);
    asio::ip::tcp::resolver::results_type endpoints = 
        resolver.resolve("127.0.0.1", "3777");

    asio::ip::tcp::socket socket(io_context);
    asio::connect(socket, endpoints);

    return 0;
}