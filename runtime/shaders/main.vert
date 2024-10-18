#version 450

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec4 v_Color;
layout(location = 2) in vec2 v_TexCoord;
layout(location = 3) in uint v_TexInd;

struct Camera {
  vec2  pos;
  vec2  view;
  float zoom;
};

layout(binding = 0) uniform UBO {
  Camera cam;
} ubo;

layout(location = 0) out vec4 f_Color;
layout(location = 1) out vec2 f_TexCoord;
layout(location = 2) out uint f_TexInd;

void main() {
  vec3 camPos = v_Position - vec3(ubo.cam.pos, 0.0f);

  camPos.x /= ubo.cam.view.x;
  camPos.y /= ubo.cam.view.y;

  gl_Position = vec4(camPos * ubo.cam.zoom, 1.0f);

  f_Color = v_Color.abgr;
  f_TexCoord = v_TexCoord;
  f_TexInd = v_TexInd;
}
