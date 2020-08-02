#!/bin/bash
for f in ./example/*.cpp; do
    g++ $@ -L./build -I./library/include $f -ldpi -o build/$(basename $f .cpp)
done

