#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
echo $SCRIPT_DIR
pushd "$SCRIPT_DIR/.."

if [[ "$1" == "--clang" ]]; then
    ./tools/linux/premake5 gmake2 --file="$SCRIPT_DIR/premake5.lua" --cc=clang
else
    ./tools/linux/premake5 gmake2 --file="$SCRIPT_DIR/premake5.lua"
fi

popd