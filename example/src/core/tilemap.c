#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <oe.h>

#include "core/tilemap.h"
#include "core/entity.h"

#define TILE_COLLISION_OFFSET 8.0f

// this variable represents tile origins on texture
static const struct { int x, y; }
  s_origs[TILE_TYPE_MAX_ENUM][TILE_VARIANT_MAX_ENUM] = {
  { // TILE_TYPE_FLOOR
    {  0, 16 }, // TILE_VARIANT_LEFT
    { 32, 16 }, // TILE_VARIANT_RIGHT
    { 16, 32 }, // TILE_VARIANT_BOT
    { 16,  0 }, // TILE_VARIANT_TOP
    {  0,  0 }, // TILE_VARIANT_LTOP
    { 32,  0 }, // TILE_VARIANT_RTOP
    {  0, 32 }, // TILE_VARIANT_LBOT
    { 32, 32 }, // TILE_VARIANT_RBOT
    { 16, 16 }, // TILE_VARIANT_CENTER
  },
  { // TILE_TYPE_WALL
    { 48, 16 }, // TILE_VARIANT_LEFT
    { 80, 16 }, // TILE_VARIANT_RIGHT
    { 64, 32 }, // TILE_VARIANT_BOT
    { 64,  0 }, // TILE_VARIANT_TOP
    { 48,  0 }, // TILE_VARIANT_LTOP
    { 80,  0 }, // TILE_VARIANT_RTOP
    { 48, 32 }, // TILE_VARIANT_LBOT
    { 80, 32 }, // TILE_VARIANT_RBOT
    { 64, 16 }, // TILE_VARIANT_CENTER
  },
};

int tilemap_load(const char *filename, tilemap_t *tilemap)
{
  FILE *fd = fopen(filename, "rb");
  if (!fd) return 0;

  // read width and height of the map
  fread(&tilemap->width,  sizeof(tilemap->width),  1, fd);
  fread(&tilemap->height, sizeof(tilemap->height), 1, fd);

  // allocate memory for cells
  const int size = tilemap->width * tilemap->height;

  tilemap->cells = malloc(size * sizeof(tilemap->cells[0]));

  if (!tilemap->cells) {
    fclose(fd);
    return 0;
  }

  // read cells
  fread(tilemap->cells, sizeof(tilemap->cells[0]), size, fd);

  fclose(fd);

  return 1;
}

void tilemap_free(tilemap_t *tilemap)
{
  free(tilemap->cells);
  memset(tilemap, 0, sizeof(*tilemap));
}

int tilemap_save(const char *filename, const tilemap_t *tilemap)
{
  FILE *fd = fopen(filename, "wb");
  if (!fd) return 0;

  // write width and height of the map
  fwrite(&tilemap->width,  sizeof(tilemap->width),  1, fd);
  fwrite(&tilemap->height, sizeof(tilemap->height), 1, fd);

  // wtite memory for cells
  const int size = tilemap->width * tilemap->height;

  fwrite(tilemap->cells, sizeof(tilemap->cells[0]), size, fd);

  fclose(fd);
  return 1;
}

void tilemap_draw(const tilemap_t *tilemap, u16 tex_id, camera_t cam)
{
  const int xstart = (int)cam.pos.x / TILEMAP_TILE_SIZE;
  const int ystart = (int)cam.pos.y / TILEMAP_TILE_SIZE;

  const int xend = xstart + cam.view.x / TILEMAP_TILE_SIZE;
  const int yend = ystart + cam.view.y / TILEMAP_TILE_SIZE + 1;

  for (int i = ystart; i < tilemap->height && i < yend; ++i) {
    for (int j = xstart; j < tilemap->width && j < xend; ++j) {
      const int ind = i * tilemap->width + j;

      const int variant = tilemap->cells[ind] & 0x0f;
      const int type    = tilemap->cells[ind] >> 4;

      const rect_t dst_rect = {
        .x      = j * TILEMAP_TILE_SIZE,
        .y      = i * TILEMAP_TILE_SIZE,
        .width  = TILEMAP_TILE_SIZE,
        .height = TILEMAP_TILE_SIZE,
      };

      const rect_t src_rect = {
        .x      = s_origs[type][variant].x,
        .y      = s_origs[type][variant].y,
        .width  = TILEMAP_TILE_SIZE,
        .height = TILEMAP_TILE_SIZE,
      };

      draw_texture_ext(dst_rect, src_rect, tex_id, WHITE, 0.0f, 0.0f);
    }
  }
}

int tilemap_point_hit(tilemap_t tilemap, vec2_t pos)
{
  const int x = (int)pos.x / TILEMAP_TILE_SIZE;
  const int y = (int)pos.y / TILEMAP_TILE_SIZE;

  return tilemap.cells[y * tilemap.width + x] >> 4;
}

int tilemap_hit(tilemap_t tilemap, vec2_t pos, collider_t collider)
{
  pos.x += collider.offset.x;
  pos.y += collider.offset.y - TILE_COLLISION_OFFSET;

  assert(collider.type == COLLIDER_TYPE_AABB,
         "unsupported collider type.");

  int x1 = pos.x / TILEMAP_TILE_SIZE;
  int y1 = pos.y / TILEMAP_TILE_SIZE;
  int x2 = (pos.x + collider.bounds.size.x) / TILEMAP_TILE_SIZE;
  int y2 = (pos.y + collider.bounds.size.y) / TILEMAP_TILE_SIZE;

  if (
    (tilemap.cells[x1 + y1 * tilemap.width] >> 4) ||
    (tilemap.cells[x2 + y1 * tilemap.width] >> 4) ||
    (tilemap.cells[x2 + y2 * tilemap.width] >> 4) ||
    (tilemap.cells[x1 + y2 * tilemap.width] >> 4)
  ) { return 1; }

  return 0;
}

