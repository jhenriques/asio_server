#!/usr/bin/env zsh

# Make build script work from anywhere:
PARENT_PATH=${0:a:h}
pushd $PARENT_PATH

# helpers for color
normal=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
blue=$(tput setaf 4)

# disables commands outputs:
set +x

IOS_CLANG=`xcrun --sdk iphoneos --find clang++`
IOS_SDK_PATH=`xcrun --sdk iphoneos --show-sdk-path`


# I need this to be able to build for
# mac (server), ios (client) ?


# Dependencies:
printf ":: Checking out submodules... "
didUpdate=`git submodule update --init --recursive`
printf "${green}DONE!${normal}\n"


# printf ":: Building oneTBB... "
# pushd 3rd_party/oneTBB
# if [ ! -e mac/lib/libtbb.a ]; then
#   mkdir -p build_mac
#   pushd build_mac

#     cmake -S .. -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DTBB_STRICT=OFF -DTBB_ENABLE_IPO=OFF  \
#       -DCMAKE_CXX_EXTENSIONS=OFF -DTBB_TEST=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=../mac &> /dev/null

#     cmake --build . -j12 --config Release &> /dev/null
#     cmake --install . &> /dev/null

#     if [[ ! $? -eq 0 ]]; then
#       printf "${red}MAC FAILED!${normal}\n";            
#       exit 1
#     fi

#     printf "${green}[MAC]${normal}";
#   popd
# fi
# if [ ! -e ios/lib/libtbb.a ]; then
#   mkdir -p build_ios
#   pushd build_ios

#     cmake -S .. -GXcode -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DTBB_STRICT=OFF -DTBB_ENABLE_IPO=OFF  \
#       -DCMAKE_CXX_EXTENSIONS=OFF -DTBB_TEST=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=../ios &> /dev/null

#     xcodebuild -project TBB.xcodeproj -sdk iphoneos -arch arm64 -target tbb -target tbbmalloc -configuration Release -quiet &> /dev/null
#     cmake --install . &> /dev/null

#     if [[ ! $? -eq 0 ]]; then
#       printf "${red}IOS FAILED!${normal}\n";            
#       exit 1
#     fi
#     printf "${green}[IOS]${normal}";
#   popd
# fi
# popd
# printf "${green} DONE!${normal}\n";


printf ":: Building Boost... "
pushd 3rd_party/boost

  if [ ! -e b2 ]; then
    ./bootstrap.sh &> /dev/null
  fi
  printf "${green}[B2]${normal}"

  if [ ! -e mac/lib/libboost_iostreams.dylib ]; then

    ./b2 --clean-all &> /dev/null
    ./b2 --stagedir=stage/mac-device --prefix=./mac --with-iostreams stage install &> /dev/null

    if [[ ! $? -eq 0 ]]; then
      printf "${red}MAC FAILED!${normal}\n"
      exit 1
    fi
  fi
  printf "${green}[MAC]${normal}"

  if [ ! -e ios/lib/libboost_iostreams.a ]; then
    
    ./b2 --clean-all &> /dev/null
    ./b2 -j8 -build-dir=build-ios-device toolset=clang architecture=arm address-model=64 \
      target-os=iphone binary-format=mach-o abi=aapcs \
      link=static runtime-link=static threading=multi variant=release \
      cxxflags="-stdlib=libc++ -std=c++20 -fvisibility=hidden -miphoneos-version-min=15.0" \
      --stagedir=stage/ios-device --prefix=./ios \
      --with-iostreams stage install &> /dev/null

    if [[ ! $? -eq 0 ]]; then
      printf "${red}IOS FAILED!${normal}\n"
      exit 1
    fi
  fi
  printf "${green}[IOS]${normal}"

popd
printf "${green} DONE!${normal}\n"


# TODO(jhenriques): Next is openvdb:

printf ":: Building openvdb... "
pushd 3rd_party/openvdb
if [ ! -e mac/lib/libopenvdb.a ]; then

  mkdir -p build_mac
  pushd build_mac

    cmake -S .. -DTBB_INCLUDEDIR=$PARENT_PATH/3rd_party/oneTBB/mac/include -DTBB_LIBRARYDIR=$PARENT_PATH/3rd_party/oneTBB/mac/lib \
      -DBoost_DIR=$PARENT_PATH/3rd_party/boost/mac/lib/cmake/Boost-1.91.0 \
      -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_CXX_EXTENSIONS=OFF -DOPENVDB_BUILD_UNITTESTS=OFF \
      -DUSE_BLOSC=OFF -DOPENVDB_BUILD_BINARIES=OFF -DOPENVDB_ENABLE_UNINSTALL=OFF -DCMAKE_INSTALL_PREFIX=../mac \
      -DUSE_STATIC_DEPENDENCIES=ON -DBUILD_SHARED_LIBS=ON -DUSE_BLOSC=OFF -DUSE_ZLIB=OFF &> /dev/null
    cmake --build . -j12 --config Release &> /dev/null
    cmake --install . &> /dev/null

    if [[ ! $? -eq 0 ]]; then
      printf "${red}MAC FAILED!${normal}\n"
      exit 1
    fi
  popd
fi
printf "${green}[MAC]${normal}"

if [ ! -e ios/lib/libopenvdb.a ]; then

  mkdir -p build_ios
  pushd build_ios

    # same as mac:
    cmake -S .. -GXcode -DTBB_INCLUDEDIR=$PARENT_PATH/3rd_party/oneTBB/ios/include -DTBB_LIBRARYDIR=$PARENT_PATH/3rd_party/oneTBB/ios/lib \
      -DBoost_DIR=$PARENT_PATH/3rd_party/boost/ios/lib/cmake/Boost-1.91.0 \
      -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_CXX_EXTENSIONS=OFF -DOPENVDB_BUILD_UNITTESTS=OFF \
      -DUSE_BLOSC=OFF -DOPENVDB_BUILD_BINARIES=OFF -DOPENVDB_ENABLE_UNINSTALL=OFF -DCMAKE_INSTALL_PREFIX=../ios \
      -DUSE_STATIC_DEPENDENCIES=ON -DBUILD_SHARED_LIBS=OFF -DUSE_BLOSC=OFF -DUSE_ZLIB=OFF -DBoost_USE_STATIC_RUNTIME=ON &> /dev/null

    xcodebuild -project openVDB.xcodeproj -sdk iphoneos -arch arm64 -target install \
      CODE_SIGNING_ALLOWED=NO CODE_SIGNING_REQUIRED=NO CODE_SIGN_STYLE=Manual -configuration Release -quiet &> /dev/null
    cmake --install . &> /dev/null
  popd
fi
printf "${green}[IOS]${normal}"

popd
printf "${green} DONE!${normal}\n"

