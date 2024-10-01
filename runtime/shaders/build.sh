#!/bin/bash

glslc main.vert -o main-vert.spv
glslc main.frag -o main-frag.spv

cp main-vert.spv ../../build/example/shaders
cp main-frag.spv ../../build/example/shaders
