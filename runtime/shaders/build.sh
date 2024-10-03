#!/bin/bash

glslc main.vert -o main-vert.spv
glslc main.frag -o main-frag.spv

rm -rf ../../build/example/shaders
mkdir ../../build/example/shaders

cp main-vert.spv ../../build/example/shaders
cp main-frag.spv ../../build/example/shaders
