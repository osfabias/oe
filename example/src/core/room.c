#include <oe.h>

#include "core/room.h"
#include "core/entity.h"
#include "core/tilemap.h"
#include "core/scripting.h"

#include "game/player.h"

#include "entity.h"

static entity_t s_entities[MAX_ENTITIES_COUNT] = {
  (entity_t){
    .transform = {
      .pos   = { 48.0f, 64.0f },
      .rot   = 0,
      .scale = { 1.0f, 1.0f }
    },

    .sprite = {
      .tex_id   = 0,
      .src_rect = (rect_t){ 16.0f, 112.0f, 16.0f, 16.0f },
      .anchor   = (vec2_t){ 8.0f, 16.0f },
      .depth    = 0.0f
    },

    .collider = {
      .type        = COLLIDER_TYPE_AABB,
      .offset      = { -8.0f, -16.0f },
      .bounds.size = { 16.0f, 16.0f },
    },

    .script = {
      .init_fn   = NULL,
      .update_fn = player_update,
    }
  }
};

static u16 s_entity_count = 1;
static entity_t *s_cur_entity = NULL;
static tilemap_t s_tilemap;

static camera_t s_cam = {
  .pos  = { 0.0f, 0.0f },
  .view = { 320.0f, 180.0f },
  .zoom = 1.0f,
};

void room_init(void)
{
  if (!tilemap_load("assets/tilemaps/test-room.tm", &s_tilemap))
    fatal("failed to load tilemap");

  info("room initialized");
}

void room_update(float dt)
{
  for (u16 i = 0; i < s_entity_count; ++i) {
    s_cur_entity = &s_entities[i];
    s_cur_entity->script.update_fn(s_cur_entity, dt);
  }
}

void room_draw(void)
{
  camera_set(s_cam);

  tilemap_draw(&s_tilemap, 0, s_cam);

  for (u16 i = 0; i < s_entity_count; ++i) {
    const entity_t *ent = &s_entities[i];

    draw_texture_ext(
      (rect_t){
        .x      = ent->transform.pos.x - ent->sprite.anchor.x,
        .y      = ent->transform.pos.y - ent->sprite.anchor.y,
        .width  = ent->sprite.src_rect.width,
        .height = ent->sprite.src_rect.height
      },
      ent->sprite.src_rect, 0, WHITE, 0, ent->sprite.depth
    );
  }

  camera_reset();
}

// +------------------------------------------------------------------+
// |                           scripting                              |
// +------------------------------------------------------------------+

i32 place_meeting(vec2_t pos)
{
  return tilemap_hit(s_tilemap, pos, s_cur_entity->collider);
}

