#!/bin/bash
# Builds Slime for the web with Emscripten:  ./build.sh [web|node]
#   web  -> dist/{index.html,slime.js,slime.wasm}
#   node -> dist/slime-node.js (headless; used by tests/)
set -euo pipefail
cd "$(dirname "$0")"
[ -n "${EMSDK:-}" ] || source "${EMSDK_DIR:-$HOME/emsdk}/emsdk_env.sh" >/dev/null 2>&1
target=${1:-web}
mkdir -p dist

# Turbo C++ 3.x: plain char is signed (same as clang/wasm), so no -funsigned-char.
CXXFLAGS=(-std=gnu++14 -O2 -g1 -Wno-everything -fpermissive -ferror-limit=0
          -fno-delete-null-pointer-checks -fno-strict-aliasing -ffp-contract=off
          -I. -Ishim -Iport -include compat.h)

case $target in
  web)  out=dist/slime.js ;;
  node) out=dist/slime-node.js ;;
esac
LDFLAGS=(--no-entry -sALLOW_MEMORY_GROWTH=1 -sFORCE_FILESYSTEM=1 -sNO_EXIT_RUNTIME=1
         -sEXPORTED_RUNTIME_METHODS=cwrap,HEAPU8,HEAPU32,FS -sEXPORTED_FUNCTIONS=_malloc,_free
         -sMODULARIZE=1 -sEXPORT_NAME=createSlime)
[ "$target" = node ] && LDFLAGS+=(-sENVIRONMENT=node)
em++ "${CXXFLAGS[@]}" *.CPP port/platform.cpp -o "$out" "${LDFLAGS[@]}"
[ "$target" = web ] && cp web/index.html dist/index.html
true
