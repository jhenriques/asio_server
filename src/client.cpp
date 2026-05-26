#include <iostream>
#include <asio.hpp>

#include "protocol.hpp"
#include "openvdb_utils.hpp"


namespace 
{
    asio::ip::tcp::socket *client_socket;

    asio::steady_timer *timer;
}



void callback(const asio::error_code& /*e*/)
{
    std::cout << "timer done!" << std::endl;

    // send some stuff:
    std::vector<uint8_t> message_buffer = create_text_message("Test");

    asio::async_write(*client_socket, asio::buffer(message_buffer), 
        [](std::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                std::cout << "Wrote message!" << std::endl;
            } else {
                
                std::cout << "error writing text message..." << std::endl;
                // socket_.close();
            }
        }
    );

    timer->expires_after(std::chrono::seconds(5));
    timer->async_wait(callback);
}


int main(int argc, char **argv)
{
    asio::io_context io_context;

    asio::ip::tcp::resolver resolver(io_context);
    asio::ip::tcp::resolver::results_type endpoints = 
        resolver.resolve("127.0.0.1", "47777");

    client_socket = new asio::ip::tcp::socket(io_context);
    asio::connect(*client_socket, endpoints);

    // keep io_context alive:
    timer = new asio::steady_timer(io_context, std::chrono::seconds(5));
    timer->async_wait(callback);

    io_context.run();

    // openvdb::initialize();

    // openvdb::FloatGrid::Ptr test_grid = openvdb::FloatGrid::create();
    // openvdb::FloatGrid::TreeType& tree = test_grid->tree();
    // tree.setValue(openvdb::Coord(0, 0, 0), 1.0f);
    // tree.setValue(openvdb::Coord(1, 1, 1), 2.0f);
    // tree.setValue(openvdb::Coord(2, 2, 2), 3.0f);

    // std::vector<char> grid_buffer;
    // if (write_grid_to_buffer(test_grid, grid_buffer))
    // {
    //     size_t size_written = asio::write(socket, asio::buffer(grid_buffer));
    //     std::cout << "--> wrote " << size_written << " bytes." << std::endl;
    // }

    std::cout << "client exiting..." << std::endl;

    delete timer;
    delete client_socket;

    return 0;
}