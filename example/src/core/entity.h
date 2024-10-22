#pragma once

#include <oe.h>

typedef struct entity entity_t;

typedef struct transform {
  vec2_t pos;
  float  rot;
  vec2_t scale;
} transform_t;

typedef struct sprite {
  u8     tex_id;
  rect_t src_rect;
  vec2_t anchor;
  float  depth;
} sprite_t;

typedef enum collider_type {
  COLLIDER_TYPE_AABB,
  COLLIDER_TYPE_CIRCLE
} collider_type_t;

typedef struct collider {
  collider_type_t type;

  vec2_t offset;
  union {
    vec2_t size;
    float  radius;
  } bounds;
} collider_t;

typedef void(*script_init_fn)(void);
typedef void(*script_update_fn)(entity_t *self, float dt);

/**
 * @brief Script component.
 *
 * @var script_t::init_fn
 * Function to be called on the room initialization.
 *
 * @var script_t::update_fn
 * Function to be called on every frame.
 */
typedef struct script {
  script_init_fn   init_fn;
  script_update_fn update_fn;
} script_t;

struct entity {
  transform_t transform;
  sprite_t    sprite;
  collider_t  collider;
  script_t    script;
};

