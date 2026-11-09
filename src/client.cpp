#include <iostream>
#include <asio.hpp>

#include "protocol.hpp"
#include "openvdb_utils.hpp"


namespace 
{
    asio::ip::tcp::socket *client_socket;

    asio::steady_timer *timer;

    std::vector<uint8_t> out_buffer;
}


void write_vdb_message();

void write_vdb_handler(const asio::error_code &ec, std::size_t)
{
    if (ec)
    {
        std::cout << "error writing vdb message..." << std::endl;
        return;
    }

    std::cout << "Wrote VDB message! (" << out_buffer.size() << " bytes)" << std::endl;
    
    timer->expires_after(std::chrono::seconds(5));
    timer->async_wait([](const asio::error_code &e)
    {
        if (!e)
        {
            write_vdb_message();
        }
    });
}

void write_vdb_message()
{
    asio::async_write(*client_socket, asio::buffer(out_buffer), write_vdb_handler);
}


// void callback(const asio::error_code& /*e*/)
// {
//     std::cout << "timer done!" << std::endl;

//     // send some stuff:
//     std::vector<uint8_t> message_buffer = create_text_message("Test");
//     asio::async_write(*client_socket, asio::buffer(message_buffer), 
//         [](std::error_code ec, std::size_t /*length*/)
//         {
//             if (!ec)
//             {
//                 std::cout << "Wrote message!" << std::endl;
//             }
//             else 
//             {
//                 std::cout << "error writing text message..." << std::endl;
//                 // socket_.close();
//             }
//         }
//     );

//     // send a test grid:
//     std::vector<char> grid_payload;
//     std::vector<uint8_t> grid_message;
//     create_vdb_message(grid_payload, grid_message);
//     asio::async_write(*client_socket, asio::buffer(grid_message), 
//         [](std::error_code ec, std::size_t)
//         {
//             if (!ec)
//             {
//                 std::cout << "Wrote test grid!" << std::endl;
//             }
//             else 
//             {
//                 std::cout << "error writing grid message..." << std::endl;
//             }
//         }
//     );

//     timer->expires_after(std::chrono::seconds(5));
//     timer->async_wait(callback);
// }


int main(int argc, char **argv)
{
    openvdb::initialize();

    // create a placeholder grid:
    openvdb::FloatGrid::Ptr test_grid = openvdb::FloatGrid::create();
    openvdb::FloatGrid::TreeType& tree = test_grid->tree();
    tree.setValue(openvdb::Coord(0, 0, 0), 1.0f);
    tree.setValue(openvdb::Coord(1, 1, 1), 2.0f);
    tree.setValue(openvdb::Coord(2, 2, 2), 3.0f);

    std::vector<char> grid_buffer;
    if (!write_grid_to_buffer(test_grid, grid_buffer))
    {
        std::cout << "--> Error: serializing grid." << std::endl;
        return 1;
    }

    // create a vdb message:
    create_vdb_message(grid_buffer, out_buffer);
    if (out_buffer.size() < sizeof(uint32_t) * 2)
    {
        std::cerr << "--> Error: failed to build VDB frame" << std::endl;
        return 1;
    }

    // enter sending loop:
    asio::io_context io_context;

    asio::ip::tcp::resolver resolver(io_context);
    asio::ip::tcp::resolver::results_type endpoints = 
        resolver.resolve("127.0.0.1", "47777");

    client_socket = new asio::ip::tcp::socket(io_context);
    asio::connect(*client_socket, endpoints);

    // keep io_context alive:
    timer = new asio::steady_timer(io_context);

    write_vdb_message();

    io_context.run();

    std::cout << "client exiting..." << std::endl;

    delete timer;
    delete client_socket;

    return 0;
}