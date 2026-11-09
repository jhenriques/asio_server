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

using asio::ip::tcp;

// #include "memory.cpp"


constexpr uint32_t MAX_MESSAGE_BYTES = 256 * 1024 * 1024; // 256 MB for now


// Session stuff:
struct Session
{
    asio::ip::tcp::socket socket;

    uint32_t id_buffer;

    uint32_t message_size;
    std::vector<char> data;
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
    session->socket.close();
}


void handle_message_body(Session *session)
{
    uint32_t header_host = ntohl(session->id_buffer);
    switch (header_host)
    {
    case TEXT_MESSAGE:
        {
            std::string text(session->data.begin(), session->data.end());
            std::cout << "RECEIVED MESSAGE: |" << text << "|" << std::endl;
            session_read(session);
            break;
        }
    case VDB_GRID:
        {
            try
            {
                openvdb::FloatGrid::Ptr grid = read_grid_from_buffer(session->data);
                if (!grid)
                {
                    std::cout << "VDB_GRID payload did not contain a grid." << std::endl;
                    disconnet_client(session);
                    return;
                }
                std::cout << "[VDB_GRID] payload " << session->message_size
                          << " bytes, grid mem " << grid->memUsage() << std::endl;
                session_read(session);
            }
            catch (const std::exception &error)
            {
                std::cout << "VDB_GRID decode failed: " << error.what() << std::endl;
                disconnet_client(session);
            }
            break;
        }
    default:
        {
            disconnet_client(session);
            break;
        }
    }
}


void read_message_body_handler(Session *session, const asio::error_code &error, std::size_t bytes_transferred)
{
    if (!error)
    {
        std::cout << "message body size: " << bytes_transferred << std::endl;
        handle_message_body(session);
    }
    else
    {
        disconnet_client(session);
    }
}

void read_message_size_handler(Session *session, const asio::error_code &error, std::size_t bytes_transferred)
{
    if (error)
    {
        disconnet_client(session);
        return;
    }

    uint32_t msg_size_host = ntohl(session->message_size);
    std::cout << "message size: " << msg_size_host << std::endl;

    if (msg_size_host >= MAX_MESSAGE_BYTES)
    {
        std::cout << "message exceeds " << MAX_MESSAGE_BYTES << " bytes... disconnecting!" << std::endl;
        disconnet_client(session);
        return;
    }

    if (msg_size_host == 0) 
    {
        handle_message_body(session);
        return;
    }

    async_read(session->socket, 
        asio::buffer(session->data, sizeof(char) * msg_size_host), 
        std::bind(read_message_body_handler, session, _1, _2));
}

void read_header_handler(Session *session, const asio::error_code &error, std::size_t bytes_transferred)
{
    if (error)
    {
        disconnet_client(session);
        return;
    }

    std::cout << "[HEADER] bytes read: " << bytes_transferred << std::endl;

    uint32_t header_host = ntohl(session->id_buffer);
    std::cout << "header type: " << header_host << std::endl;

    switch (header_host)
    {
    case TEXT_MESSAGE:
    case VDB_GRID:
        {
            async_read(session->socket, 
                asio::buffer(&session->message_size, sizeof(uint32_t)), 
                std::bind(read_message_size_handler, session, _1, _2));
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

void session_read(Session *session)
{
    // std::cout << "session_read: " << session->socket.remote_endpoint().address() << std::endl;

    // read the header:
    async_read(session->socket, 
        asio::buffer(&session->id_buffer, sizeof(uint32_t)),
        std::bind(read_header_handler, session, _1, _2));
}


void start_accept();

void accept_handler(const asio::error_code& error, asio::ip::tcp::socket socket)
{
    if (!error)
    {
        std::cout << "New connection from: " << socket.remote_endpoint().address() << std::endl;
        sessions.push_back({
            .socket = std::move(socket),
            .data = std::vector<char>(MAX_MESSAGE_BYTES)
        });

        session_read(&sessions.back());
    }

    start_accept();
}

void start_accept()
{
    std::cout << "start_accept()" << std::endl;
    acceptor->async_accept(std::bind(accept_handler, _1, _2));

    // asio::ip::tcp::socket *socket = new asio::ip::tcp::socket(io_context);
    // // Session new_session = ;
    // sessions.push_back({ 
    //     .socket = socket,
    //     .id_buffer = 0 });
    // std::cout << "NEW SESSION: socket: " << (uintptr_t)sessions.back().socket << std::endl;
    // acceptor->async_accept(*socket, std::bind(accept_handler, &sessions.back(), _1));
}




int main(int argc, char **argv)
{
    openvdb::initialize();

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