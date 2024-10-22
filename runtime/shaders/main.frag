#version 450

layout(location = 0) in vec4 f_Color;
layout(location = 1) flat in uint f_TexInd;
layout(location = 2) in vec2 f_TexCoord;

layout(binding = 1) uniform sampler2D texSamplers[3];

layout(location = 0) out vec4 fragColor;

void main() {
  ivec2 texSize = textureSize(texSamplers[f_TexInd], 0);

  vec2 resTexCoord = f_TexCoord;

  resTexCoord.x /= texSize.x;
  resTexCoord.y /= texSize.y;

  fragColor = texture(texSamplers[f_TexInd], resTexCoord).bgra * f_Color;
}

