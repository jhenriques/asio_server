#pragma once

#if defined WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif


typedef enum 
{
    INVALID = 0,
    TEXT_MESSAGE,
    VDB_GRID,
} message_type;


struct message_header
{
    uint32_t id;
};

struct text_message
{
    message_header header;

    uint32_t size;
    char *c_str;
};


message_header read_message_header(void *data, size_t size)
{
    message_header result{INVALID};

    if (size >= sizeof(message_header))
    {
        memcpy(&result, data, sizeof(message_header));
    }

    return result;
}

text_message read_text_message(void *data, size_t size)
{
    text_message result; // {0, nullptr};



    return result;
}


std::vector<uint8_t> create_text_message(std::string msg)
{
    std::vector<uint8_t> result;
    result.reserve(sizeof(message_header) + sizeof(uint32_t) + msg.size());

    // write the header:
    uint32_t header_net = htonl((uint32_t)TEXT_MESSAGE);
    // std::cout << "header_net: " << header_net << std::endl;

    uint8_t *header_bytes = reinterpret_cast<uint8_t *>(&header_net);
    result.insert(result.end(), header_bytes, header_bytes + 4);
    // std::cout << "result: " << *((uint32_t *)result.data()) << std::endl;

    // write the message:
    uint32_t msg_size_net = htonl(msg.length());
    uint8_t *msg_size_bytes = reinterpret_cast<uint8_t *>(&msg_size_net);
    result.insert(result.end(), msg_size_bytes, msg_size_bytes + 4);
    result.insert(result.end(), msg.begin(), msg.end());

    return result;
}

errno_t create_vdb_message(const std::vector<char> &payload, std::vector<uint8_t> &out_buffer)
{
    out_buffer.reserve(sizeof(uint32_t) + sizeof(uint32_t) + payload.size());

    // Write the header:
    uint32_t header_net = htonl((uint32_t)VDB_GRID);
    std::cout << "header_net: " << header_net << std::endl;

    uint8_t *header_bytes = reinterpret_cast<uint8_t *>(&header_net);
    out_buffer.insert(out_buffer.end(), header_bytes, header_bytes + 4);
    std::cout << "out_buffer: " << *((uint32_t *)out_buffer.data()) << std::endl;

    // write the grid size:
    uint32_t msg_size_net = htonl(static_cast<uint32_t>(payload.size()));
    uint8_t *msg_size_bytes = reinterpret_cast<uint8_t *>(&msg_size_net);
    out_buffer.insert(out_buffer.end(), msg_size_bytes, msg_size_bytes + 4);
    
    // write the grid:
    const uint8_t *payload_bytes = reinterpret_cast<const uint8_t *>(payload.data());
    out_buffer.insert(out_buffer.end(), payload_bytes, payload_bytes + payload.size());

    return 0;
}