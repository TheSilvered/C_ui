#! /usr/bin/sh

SCRIPT_DIR="$(dirname -- $(readlink -f -- "$0"))"
cd "$SCRIPT_DIR"

INSTALL_PATH="$(./SDLpath.sh)"

if [ -e "$INSTALL_PATH" ]; then
    exit 0
fi

SDL_VERSION="$(./SDLpath.sh version)"
SYS_INFO="$(./SDLpath.sh system)"

SRC_DIR="SDL"
GIT_REPO="https://github.com/libsdl-org/SDL.git"

if [ -e "$SRC_DIR" ]; then
    cd "$SRC_DIR"
    git checkout "release-$SDL_VERSION"
else
    git clone -b "release-$SDL_VERSION" --depth 1 "$GIT_REPO" "$SRC_DIR"
    cd "$SRC_DIR"
fi

echo "Building SDL $SDL_VERSION for $SYS_INFO"

rm -rf build
mkdir build
cd build
cmake ..
cmake --build . --config RelWithDebInfo
cmake --install . --config RelWithDebInfo --prefix "$INSTALL_PATH"
