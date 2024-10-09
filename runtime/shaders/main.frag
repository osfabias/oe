#version 450

layout(location = 0) in vec4 f_Color;
layout(location = 1) in vec2 f_TexCoord;
layout(location = 2) flat in uint f_TexInd;

layout(binding = 1) uniform sampler2D texSampler[4];

layout(location = 0) out vec4 fragColor;

void main() {
  fragColor = texture(texSampler[f_TexInd], f_TexCoord) * f_Color;
}
