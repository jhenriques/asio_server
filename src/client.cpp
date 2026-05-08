#include <iostream>
#include <asio.hpp>

#include "openvdb_utils.hpp"

int main(int argc, char **argv)
{
    asio::io_context io_context;

    asio::ip::tcp::resolver resolver(io_context);
    asio::ip::tcp::resolver::results_type endpoints = 
        resolver.resolve("127.0.0.1", "47777");

    asio::ip::tcp::socket socket(io_context);
    asio::connect(socket, endpoints);


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

    return 0;
}