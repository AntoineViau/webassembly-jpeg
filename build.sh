emcc jpeg-read.c jpeg-write.c webassembly-jpeg.c jpeg-9c/.libs/libjpeg.a \
    -s WASM=1 -s NO_EXIT_RUNTIME=1 -o webassembly-jpeg.js \
    -s 'EXTRA_EXPORTED_RUNTIME_METHODS=["cwrap", "writeArrayToMemory","getValue"]'
