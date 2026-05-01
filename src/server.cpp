#include <iostream>
#include <asio.hpp>
#include <vector>

#include "openvdb_utils.hpp"

// #include "memory.cpp"

// Session stuff:
struct Session
{
    asio::ip::tcp::socket *socket;
};


namespace
{
    asio::io_context *io_context = nullptr;
    asio::ip::tcp::acceptor *acceptor = nullptr;

    openvdb::FloatGrid::Ptr test_grid;

    std::vector<char> grid_buffer;

    std::vector<Session> sessions;
}

void session_read(Session &session);

void read_handler(const asio::error_code &error, std::size_t bytes_transferred)
{
    if (!error)
    {
        std::cout << " bytes read: " << bytes_transferred << std::endl;
        session_read(session);
    }
    else
    {
       std::cout << "Participant disconnected." << std::endl; 
    }
}

void session_read(Session &session)
{
    asio::streambuf buffer;
    async_read(session.socket, buffer, read_handler);
}



void start_accept();

void accept_handler(asio::ip::tcp::socket *socket, const asio::error_code& error)
{
    if (!error)
    {
        std::cout << "accepted connection!" << std::endl;

        sessions.push_back({.socket = std::move(socket)})

        session_read(session);
    }

    start_accept();
}

void start_accept()
{
    asio::ip::tcp::socket socket(*io_context);
    acceptor->async_accept(socket, std::bind(accept_handler, &socket, asio::placeholders::error));
}




int main(int argc, char **argv)
{
    // test for now: 
    openvdb::initialize();

    test_grid = openvdb::FloatGrid::create();
    openvdb::FloatGrid::TreeType& tree = test_grid->tree();
    tree.setValue(openvdb::Coord(0, 0, 0), 1.0f);
    tree.setValue(openvdb::Coord(1, 1, 1), 2.0f);
    tree.setValue(openvdb::Coord(2, 2, 2), 3.0f);

    if (write_grid_to_buffer(test_grid, grid_buffer))
    {
        std::cout << "--> created a test grid with " << test_grid->memUsage() << " bytes." << std::endl;
    }
    


    // Read it back:
    openvdb::FloatGrid::Ptr read_grid = read_grid_from_buffer(grid_buffer);
    if (read_grid)
    {
        std::cout << "--> read grid with " << openvdb::tools::memUsageIfLoaded(read_grid->tree()) << " bytes." << std::endl;
    }

    // std::istringstream istr(std::string(grid_buffer.begin(), grid_buffer.end()), std::ios_base::binary);
    // openvdb::GridPtrVecPtr gridsRead = openvdb::io::Stream(istr).getGrids();
    // if (!gridsRead || gridsRead->empty())
    // {
    //     std::cerr << "ERROR: No grids were read back!" << std::endl;
    // }
    // else
    // {
    //     openvdb::FloatGrid::Ptr gridRead = openvdb::gridPtrCast<openvdb::FloatGrid>((*gridsRead)[0]);

    //     if (test_grid == gridRead)
    //     {
    //         std::cout << "--> read back the same grid" << std::endl;
    //     }
    // }


    io_context = new asio::io_context();
    acceptor = new asio::ip::tcp::acceptor(*io_context, 
        asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 3777));

    start_accept();

    io_context->run();

    return 0;
}