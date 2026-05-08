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

rem Build boost:
if NOT EXIST .\3rd_party\boost\win32\lib\libboost_iostreams-vc145-mt-x64-1_91.lib (
    echo need to build boost...
    pushd 3rd_party\boost

        if NOT EXIST b2.exe (
            echo need to build b2...
            bootstrap.bat
        )

        .\b2 --clean-all
        .\b2 --stagedir=stage/win32 --prefix=./win32 --with-iostreams stage install

    popd
)


rem Build openvdb
if NOT EXIST .\3rd_party\openvdb\bin\openvdb.dll (
    echo need to build openvdb...
    pushd 3rd_party\openvdb
        
        mkdir build
        pushd build

        cmake -S .. -DTBB_INCLUDEDIR=d:\code\asio_server\3rd_party\oneTBB\include -DTBB_LIBRARYDIR=d:\code\asio_server\3rd_party\oneTBB\lib -DBoost_DIR=d:\code\asio_server\3rd_party\boost\win32\lib\cmake\Boost-1.91.0 -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DOPENVDB_BUILD_UNITTESTS=OFF -DUSE_BLOSC=OFF -DUSE_ZLIB=OFF -DOPENVDB_BUILD_BINARIES=OFF -DOPENVDB_ENABLE_UNINSTALL=OFF -DCMAKE_INSTALL_PREFIX=..
        cmake --build . -j12 --config Release
        cmake --install .

        popd
    popd
)


mkdir %BUILD_FOLDER%
pushd %BUILD_FOLDER%

set LINKER_OPTIONS=/link /NOLOGO /LIBPATH:../3rd_party/oneTBB/lib /LIBPATH:../3rd_party/openvdb/lib tbb12.lib openvdb.lib


echo "### Building server ###"
cl /EHsc /std:c++17 /MT -I../3rd_party/asio/include -I../3rd_party/oneTBB/include -I../3rd_party/openvdb/include -D_WIN32_WINNT=0x0601 %SRC_FOLDER%/server.cpp %LINKER_OPTIONS%

echo "### Building client ###"
cl /EHsc /std:c++17 /MT -I../3rd_party/asio/include -I../3rd_party/oneTBB/include -I../3rd_party/openvdb/include -D_WIN32_WINNT=0x0601 %SRC_FOLDER%/client.cpp %LINKER_OPTIONS%

cp ../3rd_party/oneTBB/bin/*.dll .
cp ../3rd_party/openvdb/bin/*.dll .

popd