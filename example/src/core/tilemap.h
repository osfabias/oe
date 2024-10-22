#pragma once

#include <oe.h>

#include "core/entity.h"

#define TILEMAP_TILE_SIZE 16

typedef enum tile_type {
  TILE_TYPE_FLOOR,
  TILE_TYPE_WALL,

  TILE_TYPE_MAX_ENUM
} tile_type_t;

typedef enum tile_variant {
  TILE_VARIANT_LEFT,
  TILE_VARIANT_RIGHT,
  TILE_VARIANT_BOT,
  TILE_VARIANT_TOP,
  TILE_VARIANT_LTOP,
  TILE_VARIANT_RTOP,
  TILE_VARIANT_LBOT,
  TILE_VARIANT_RBOT,
  TILE_VARIANT_CENTER,

  TILE_VARIANT_MAX_ENUM
} tile_variant_t;

typedef struct tilemap {
  int width;
  int height;

  /* cell format: [type: 4b | variant: 4b] x 4 */
  char     *cells;
} tilemap_t;

int  tilemap_load(const char *filename, tilemap_t *tilemap);
int  tilemap_save(const char *filename, const tilemap_t *tilemap);
void tilemap_free(tilemap_t *tilemap);

void tilemap_calc_variants(tilemap_t *tilemap);

void tilemap_draw(
  const tilemap_t *tilemap, u16 tex_id, camera_t cam
);

int tilemap_point_hit(tilemap_t tilemap, vec2_t pos);

int tilemap_hit(tilemap_t tilemap, vec2_t pos, collider_t collider);

