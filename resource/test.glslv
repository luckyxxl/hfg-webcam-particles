#version 330 core

in vec2 v_position;
in vec3 v_rgb;

out vec3 color;

void main() {
  gl_Position = vec4(v_position * vec2(2.) - vec2(1.), 0., 1.);
  color = v_rgb;
  gl_PointSize = 8.;
}
