#include <iostream>
#include <asio.hpp>
#include <vector>

#if defined WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "openvdb_utils.hpp"

#include "protocol.hpp"

using namespace std::placeholders;

// #include "memory.cpp"

// Session stuff:
struct Session
{
    asio::ip::tcp::socket *socket = nullptr;
    char data[1024];
};


namespace
{
    asio::io_context io_context;
    asio::ip::tcp::acceptor *acceptor = nullptr;

    openvdb::FloatGrid::Ptr test_grid;

    std::vector<char> grid_buffer;

    std::vector<Session> sessions = {};
}

void session_read(Session *session);

void disconnet_client(Session *session)
{
    std::cout << "Participant disconnected." << std::endl;
   session->socket->close();
   free(session->socket);
}

void read_text_message_header_handler(Session *session, const asio::error_code &error, std::size_t bytes_transferred)
{
    if (!error)
    {
        std::cout << "[TEXT_MESSAGE] message size: " << bytes_transferred << std::endl;
        // async_read(*session.socket, asio::buffer(session.data, sizeof(message_header)), std::bind(read_header_handler, session, _1, _2));

        session_read(session);
    }
    else
    {
        disconnet_client(session);
    }
}

void read_header_handler(Session *session, const asio::error_code &error, std::size_t bytes_transferred)
{
    if (!error)
    {
        std::cout << "[HEADER] bytes read: " << bytes_transferred << std::endl;

        uint32_t *header_net = reinterpret_cast<uint32_t *>(session->data);
        uint32_t header_host = ntohl(*((uint32_t *)header_net));
        std::cout << "header type: " << header_host << std::endl;

        switch (header_host)
        {
        case TEXT_MESSAGE:
            {
                async_read(*session->socket, asio::buffer(session->data, sizeof(uint32_t)), std::bind(read_text_message_header_handler, session, _1, _2));
                break;
            }
        default:
            {
                std::cout << "non-supported protocol... disconnecting!" << std::endl;
                disconnet_client(session);
                break;
            }
        }
    }
    else
    {
        disconnet_client(session);
    }
}

void session_read(Session *session)
{
    // read the header:
    async_read(*session->socket, asio::buffer(session->data, sizeof(message_header)), std::bind(read_header_handler, session, _1, _2));
}


void start_accept();

void accept_handler(Session *session, asio::ip::tcp::socket *socket, const asio::error_code& error)
{
    std::cout << "accept_handler()" << std::endl;

    if (!error)
    {
        std::cout << "accepted connection!" << std::endl;

        session_read(session);
    }

    start_accept();
}

void start_accept()
{
    std::cout << "start_accept()" << std::endl;

    asio::ip::tcp::socket *socket = new asio::ip::tcp::socket(io_context);
    sessions.push_back({ socket });

    std::cout << "NEW SESSION: " << (uintptr_t)sessions.back().data << std::endl;
    
    acceptor->async_accept(*socket, std::bind(accept_handler, &sessions.back(), socket, _1));
}




int main(int argc, char **argv)
{
    // // test for now: 
    // openvdb::initialize();

    // test_grid = openvdb::FloatGrid::create();
    // openvdb::FloatGrid::TreeType& tree = test_grid->tree();
    // tree.setValue(openvdb::Coord(0, 0, 0), 1.0f);
    // tree.setValue(openvdb::Coord(1, 1, 1), 2.0f);
    // tree.setValue(openvdb::Coord(2, 2, 2), 3.0f);

    // if (write_grid_to_buffer(test_grid, grid_buffer))
    // {
    //     std::cout << "--> created a test grid with " << test_grid->memUsage() << " bytes." << std::endl;
    // }
    
    // // Read it back:
    // openvdb::FloatGrid::Ptr read_grid = read_grid_from_buffer(grid_buffer);
    // if (read_grid)
    // {
    //     std::cout << "--> read grid with " << openvdb::tools::memUsageIfLoaded(read_grid->tree()) << " bytes." << std::endl;
    // }

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


    // io_context = new asio::io_context();
    acceptor = new asio::ip::tcp::acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 47777));

    start_accept();

    io_context.run();

    std::cout << "server exiting..." << std::endl;
    
    free(acceptor);
    // free(io_context);

    // for (size_t i = 0, iCount = sessions.size(); i < iCount; ++i)
    // {
    //     if (sessions[i].socket)
    //     {
    //         free(sessions[i].socket);
    //     }
    // }

    return 0;
}