#version 330 core

uniform float time;
uniform float globalEffectTime;

in vec2 v_position;
in vec3 v_rgb;
in vec3 v_hsv;
in float v_localEffectStrength;

out vec3 color;

const float PI = 3.14159265;

vec2 getDirectionVector(float angle) {
  return vec2(cos(angle), sin(angle));
}

void main() {
  vec3 initialPosition = vec3(v_position * vec2(2.) - vec2(1.), 0.);
  vec3 position = initialPosition;
  {
    float offset = (-cos(time * PI) + 1.) / 2. * v_localEffectStrength * .2;
    position.xy += offset * getDirectionVector(v_hsv.x) * .1;
  }
  {
    vec2 target = getDirectionVector(v_hsv.x + globalEffectTime * .25) * vec2(.8);

    vec2 d = target - initialPosition.xy;
    float d_len = length(d);

    float stop_t = sqrt(2. * d_len / 2.);

    vec2 result;

    if(globalEffectTime < stop_t) {
      float t = min(globalEffectTime, stop_t);
      result = .5 * d / d_len * 2. * t * t;
    } else if(globalEffectTime < 1.5) {
      result = d;
    } else {
      float t = globalEffectTime - 1.5;
      //result = mix(d, vec2(0.), 1. - (1.-t) * (1.-t));
      //result = mix(d, vec2(0.), t * t);
      result = mix(d, vec2(0.), -cos(t / 1.5 * PI) * .5 + .5);
    }

    position.xy += result;
  }
  gl_Position = vec4(position, 1.);
  color = v_rgb;
  gl_PointSize = 4.;
}
