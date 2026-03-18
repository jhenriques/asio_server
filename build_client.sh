#!/usr/bin/env zsh

### Make build script work from anywhere:
PARENT_PATH=${0:a:h}
pushd $PARENT_PATH

### Main tweakables:
TARGET_NAME="client"
BUILD_FOLDER="build"
SRC_FOLDER="../src"

### Dependencies:
echo "### Checking out submodules (this might take a second for the first time...) ###"
didUpdate=`git submodule update --init --recursive`


echo "### Building server ###"
if [ ! -d "$BUILD_FOLDER" ]; then
    mkdir -p $BUILD_FOLDER
fi
pushd $BUILD_FOLDER

SOURCE_FILES=(
    'client.cpp'                        'macOS_client.o'
)

### Build main
printf "\n### Compiling client:\n"
MAC_CLANG=`xcrun --sdk macosx --find clang++`
MAC_SDK_PATH=`xcrun --sdk macosx --show-sdk-path`

MAC_BUILD_TARGET=arm64
MAC_TARGET=arm64-apple-darwin22.1.0

BUILD_TYPE=Release
BUILD_FLAGS=(-O3 -ffast-math -flto)
LINKING_FLAGS=(-flto)
# if [ "$3" = "debug" ]; then
#     BUILD_TYPE=Debug
#     BUILD_FLAGS=(-g)
#     LINKING_FLAGS=(-Wl)
# fi
LINKING_FLAGS+=(-Wl -rpath @executable_path/.)
LINKING_FLAGS+=(-Wl -rpath @executable_path/../Frameworks/.)

rm -f *.o
rm -f $TARGET_NAME

BUILD_STATUS=0
OBJECT_FILES=()
pids=()
start_time=$(date +%s)
for key value in ${(kv)SOURCE_FILES}; do
    echo "Compiling $key ($MAC_BUILD_TARGET - $BUILD_TYPE) ..."
    env $MAC_CLANG ${=BUILD_FLAGS} -std=c++17 -fPIC -fobjc-arc -isysroot $MAC_SDK_PATH --target=$MAC_TARGET \
        -I../3rd_party/asio-1.36.0/include/ \
        -Wno-format-security -c $SRC_FOLDER/$key -o $value &
    pids+=($!)
    if [ $? -ne 0 ]; then
        BUILD_STATUS=1
    fi
    OBJECT_FILES+=($value)
done
for pid in "${pids[@]}"; do
    if [[ $pid -le 0 ]] || ! ps -p $pid > /dev/null; then
        continue
    fi
    if ! wait "$pid"; then
        BUILD_STATUS=1
        printf "Build status: \033[0;31mFailed\033[0m\n"
        exit 1
    fi
done

end_time=$(date +%s)
time_taken=$((end_time - start_time))
echo "Time taken: $time_taken seconds"

echo "Linking "$TARGET_NAME"_"$MAC_BUILD_TARGET" ($BUILD_TYPE) ..."
env $MAC_CLANG ${=BUILD_FLAGS} ${=LINKING_FLAGS} -std=c++17 -fPIC -fobjc-arc -isysroot $MAC_SDK_PATH -framework CoreGraphics -framework Cocoa -framework Metal -framework MetalKit -framework Symbols --target=$MAC_TARGET \
    $OBJECT_FILES -o $TARGET_NAME
if [ $? -ne 0 ]; then
    BUILD_STATUS=1
fi

if [ "$BUILD_TYPE" = "Debug" ]; then
    # Only enable this if debug symbols are required:
    env dsymutil $TARGET_NAME
fi

if [ $BUILD_STATUS -eq 0 ]; then
    printf "Build status: \033[0;32mSuccess\033[0m\n"
else
    printf "Build status: \033[0;31mFailed\033[0m\n"
fi

mkdir -p ../binaries
cp $TARGET_NAME ../binaries/$TARGET_NAME

popd #$BUILD_FOLDER
popd #$PARENT_PATH
