# ~ this lib build as as separate lib, because compilation fails with
# ~ all -Wextra -Wpedantic flags...

add_library(stb_image STATIC stb_image.c)
target_link_directories(stb_image PUBLIC .)
