#version 450

layout(location = 0) in vec4 f_Color;

layout(location = 0) out vec4 fragColor;

void main() {
  fragColor = f_Color;
}