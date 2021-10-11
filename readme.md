# Experiments : read and write Jpeg with Web Assembly in real time

To have a look right now : http://antoineviau.com/webassembly-jpeg/index.html


## What does it do ? 
When index.html is launched in your browser, a Jpeg image is fetched() and processed as raw data (see `loadSrcImage`). The data are given to the C/WASM code through `setSrcImage` : it will decode the raw data as Jpeg and store the bitmap in memory for further Jpeg encoding/decoding.  
After this init step, when the user moves the slider, the `update` JS function will call the `compress` C/WASM function.  
This is the app core : the image is encoded with the given quality value, then decoded and send back to the Javascript. Finally, the image is displayed in a canvas.  

**Have a look to the code (especially `index.html`), almost every line is commented.**

## Install, build and run

Install Emscripten from https://kripken.github.io/emscripten-site/index.html  

Install dependencies : 

    npm install 
 
Let's build our app :

    cd ..
    emcc -o webassembly-jpeg.js jpeg-read.c jpeg-write.c webassembly-jpeg.c -s USE_LIBJPEG -O3 -s WASM=1 -s NO_EXIT_RUNTIME=1 -s 'EXPORTED_RUNTIME_METHODS=["writeArrayToMemory","getValue", "cwrap"]' -s EXPORTED_FUNCTIONS='['_malloc', '_free']'

or run the build script :

    ./build.sh

Launch a local server : 

    npm start

And play with the app on `localhost:8080/index.html`

