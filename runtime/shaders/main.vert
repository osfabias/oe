#version 450

layout(location = 0) in vec2 v_Position;
layout(location = 1) in vec4 v_Color;
layout(location = 2) in vec2 v_TexCoord;

layout(location = 0) out vec4 f_Color;

void main() {
  gl_Position = vec4(v_Position, 0.0f, 1.0f);

  f_Color = v_Color;
}
