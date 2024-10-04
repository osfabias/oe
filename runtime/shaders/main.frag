#version 450

layout(location = 0) in vec4 f_Color;
layout(location = 1) in vec2 f_TexCoord;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 fragColor;

void main() {
  fragColor = texture(texSampler, f_TexCoord) * f_Color;
}
