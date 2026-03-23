@echo off
setlocal

set BUILD_FOLDER="build"
set SRC_FOLDER="../src"

rem Dependencies:
echo ### Checking out submodules (this might take a second for the first time...) ###
git submodule update --init --recursive


rem Build oneTBB
if NOT EXIST .\3rd_party\oneTBB\bin\tbb12.dll (
    echo need to build oneTBB...
    pushd 3rd_party\oneTBB

        mkdir build
        pushd build

        cmake -S .. -DTBB_TEST=OFF -DCMAKE_INSTALL_PREFIX=..
        cmake --build . -j12 --config Release
        cmake --install .

        popd
    popd
)

rem Build openvdb
if NOT EXIST .\3rd_party\openvdb\bin\openvdb.dll (
    echo need to build openvdb...
    pushd 3rd_party\openvdb
        
        mkdir build
        pushd build

        cmake -S .. -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DOPENVDB_BUILD_UNITTESTS=OFF -DOPENVDB_BUILD_PYTHON_WHEELS=OFF -DOPENVDB_BUILD_BOOST_UNITTESTS=OFF -DoneTBB_DIR=..\..\oneTBB\ -DUSE_BLOSC=OFF -DOPENVDB_BUILD_BINARIES=OFF -DOPENVDB_ENABLE_UNINSTALL=OFF -DCMAKE_INSTALL_PREFIX=..
        cmake --build . -j12 --config Release
        cmake --install .

        popd
    popd
)



rem mkdir %BUILD_FOLDER%
rem pushd %BUILD_FOLDER%

rem echo "### Building server ###"
rem cl /EHsc -I../3rd_party/asio/include -D_WIN32_WINNT=0x0601 %SRC_FOLDER%/server.cpp

rem echo "### Building client ###"
rem cl /EHsc -I../3rd_party/asio/include -D_WIN32_WINNT=0x0601 %SRC_FOLDER%/client.cpp

rem popd