#include "oe.h"

vec2_t vec2_add(vec2_t v1, vec2_t v2) {
  v1.x += v2.x;
  v1.y += v2.y;
  return v1;
}

vec2_t vec2_sub(vec2_t v1, vec2_t v2) {
  v1.x -= v2.x;
  v1.y -= v2.y;
  return v1;
}

