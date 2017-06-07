#version 330 core

in vec3 color;

out vec4 fragColor;

void main() {
  float v = pow(max(1. - 2. * length(gl_PointCoord - vec2(.5)), 0.), 1.5);
  fragColor = vec4(color * v, 0.);
}
