#include <string.h>

#include <opl.h>

#include "oe.h"
#include "internal.h"

const opl_input_state_t *s_cur_input_state;
opl_input_state_t s_prev_input_state;

void _input_init(void) {
  s_cur_input_state = opl_get_input_state();
  memcpy(&s_prev_input_state, s_cur_input_state,
         sizeof(opl_input_state_t));
}

void _input_update(void) {
  memcpy(&s_prev_input_state, s_cur_input_state,
         sizeof(opl_input_state_t));
}

int is_key_pressed(key_t key) {
  return s_cur_input_state->keys[key] && !s_prev_input_state.keys[key];
}

int is_key_down(key_t key) {
  return s_cur_input_state->keys[key];
}

int is_key_released(key_t key) {
  return !s_cur_input_state->keys[key] && s_prev_input_state.keys[key];
}

int is_btn_pressed(btn_t btn) {
  return s_cur_input_state->btns[btn] && s_prev_input_state.btns[btn];
}

int is_btn_down(btn_t btn) {
  return s_cur_input_state->btns[btn];
}

int is_btn_released(btn_t btn) {
  return !s_cur_input_state->btns[btn] && s_prev_input_state.btns[btn];
}

vec2_t mouse_pos(void) {
  return (vec2_t){ s_cur_input_state->x, s_cur_input_state->y };
}

float mouse_wheel(void) {
  return s_cur_input_state->wheel;
}

