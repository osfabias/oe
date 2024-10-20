#include "core/entity.h"
#include "core/scripting.h"

#include "game/player.h"

void player_update(entity_t *self, float dt) {
  i32 haxis = is_key_down(KEY_D) - is_key_down(KEY_A);
  i32 vaxis = is_key_down(KEY_S) - is_key_down(KEY_W);

  self->transform.pos.x += haxis * 50.0f * dt;
  while (place_meeting(self->transform.pos))
    self->transform.pos.x -= haxis * dt;

  self->transform.pos.y += vaxis * 50.0f * dt;
  while (place_meeting(self->transform.pos))
    self->transform.pos.y -= vaxis * dt;
}

