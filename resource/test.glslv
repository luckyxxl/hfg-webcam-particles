#version 330 core

uniform float time;

in vec2 v_position;
in vec3 v_rgb;
in vec3 v_hsv;

out vec3 color;

const float PI = 3.14159265;

vec2 getDirectionVector(float angle) {
  return vec2(cos(angle), sin(angle));
}

void main() {
  vec3 position = vec3(v_position * vec2(2.) - vec2(1.), 0.);
  {
    float offset = (-cos(time * PI) + 1.) / 2.;
    position.xy += offset * getDirectionVector(v_hsv.x) * .1;
  }
  gl_Position = vec4(position, 1.);
  color = v_rgb;
  gl_PointSize = 4.;
}
