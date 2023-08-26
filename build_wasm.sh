#!/bin/bash

rm -rf ./dist
mkdir ./dist
export MEMORY=67108864
export FUNCTIONS="['_open_decoder','_decode_data','_malloc','_free']"

emcc ff_decode_video.c ffmpeg/lib/libavcodec.a ffmpeg/lib/libavutil.a \
     -I "ffmpeg/include" \
     --no-entry \
     -s TOTAL_MEMORY=${MEMORY} \
     -s EXPORTED_FUNCTIONS=${FUNCTIONS} \
     -s EXPORTED_RUNTIME_METHODS="['addFunction']" \
     -s ALLOW_TABLE_GROWTH \
     -O2 \
     -o ./dist/libffmpeg.js
