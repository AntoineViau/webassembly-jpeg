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

Download the JPEG lib from the Independant Jpeg Group website to project directory :

    http://www.ijg.org/files/

Let's say you downloaded `jpegsrc.v9b.tar.gz`

Untar & unzip the jpeg lib : 

    mkdir libjpeg
    tar xvzf jpegsrc.v9b.tar.gz -C ./libjpeg --strip-components=1

The first step is to configure the Jpeg lib build environment. Usually, you would launch the `configure` script, but since our target is not the host architecture/operating system but WASM, we use `emconfigure` to wrap this process : 

    cd libjpeg
    emconfigure ./configure
    # You may have an error message about the dynamic linker. Just ignore it.

We can now build the library in WASM format. We use the `emmake` wrapper : 
    
    emmake make

We have now a WASM Jpeg library ready to be included into our project.  
Let's build our app :

    cd ..
    emcc -o webassembly-jpeg.js jpeg-read.c jpeg-write.c webassembly-jpeg.c libjpeg/.libs/libjpeg.a -O3 -s WASM=1 -s NO_EXIT_RUNTIME=1 -s 'EXTRA_EXPORTED_RUNTIME_METHODS=["writeArrayToMemory","getValue", "cwrap"]' 


Launch a local server : 

    npm start

And play with the app on `localhost:8080/index.html`

