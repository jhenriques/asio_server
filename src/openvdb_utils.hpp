#pragma once

// @author: José Henriques 2026
// All rights belong to the author.

#include <openvdb/openvdb.h>
#include <openvdb/io/Stream.h>

// TODO(jhenriques): At some point I should fix nanovdb to make it more efficient
// to (de)serialize the grids...
// #include <nanovdb/tools/NanoToOpenVDB.h>
// #include <nanovdb/tools/CreateNanoGrid.h>


// Grid de/serialize:
// The idea is to have the helpers for de/serialize voxel grids here
// I will keep 2 versions around, one that converts to nanovdb to optimize the network transfer
// but requiring a possibly costly conversion at sending and receiving.
// And another version that does not ever converts to nanovdb, which might have a worse network
// performace (bigger byte array?) but required no conversion.


// bool write_grid_to_buffer_nano(openvdb::FloatGrid::Ptr grid, std::vector<char> &buffer)
// {
//     buffer.clear();

//     // // Convert once (you can keep the handle)
//     // nanovdb::GridHandle<nanovdb::FloatGrid> handle = nanovdb::tools::openToNanoVDB(grid);
//     // nanovdb::GridHandle<nanovdb::HostBuffer> handle = nanovdb::createNanoGrid(*grid);
//     auto handle = nanovdb::tools::createNanoGrid(*grid);

//     // // Get raw pointer + exact size
//     // const char* data = reinterpret_cast<const char*>(handle.data());
//     // size_t size_bytes = handle.size();   // or handle.gridSize() for the grid only

//     // buffer.resize(size_bytes);
//     // memcpy(buffer.data(), data, size_bytes);

//     return true;
// }

// TODO(jhenriques): this is probably not a float grid in the final version:
// NOTE(jhenriques): input buffer will be cleared if gris is valid (not null)
bool write_grid_to_buffer(openvdb::FloatGrid::Ptr grid, std::vector<char> &buffer)
{
    if (!grid)
    {
        return false;
    }

    buffer.clear();

    std::ostringstream ostr(std::ios_base::binary);   // binary mode is required
    openvdb::io::Stream(ostr).write({grid});    // TODO(jhenriques): this shit throws...

    std::string tmp_str = ostr.str();
    buffer.insert(buffer.begin(), tmp_str.begin(), tmp_str.end());

    return true;
}

openvdb::FloatGrid::Ptr read_grid_from_buffer(std::vector<char> &buffer)
{
    std::istringstream istr(std::string(buffer.begin(), buffer.end()), std::ios_base::binary);

    // This is sequential and loads all grids. It is better that using readGrid()!
    openvdb::GridPtrVecPtr grids = openvdb::io::Stream(istr).getGrids();
    if (!grids || grids->empty())
    {
        return nullptr;
    }

    // For now, I know there is only one grid. Later, I might want to revise this
    return openvdb::gridPtrCast<openvdb::FloatGrid>((*grids)[0]);
}
