#version 120

attribute vec2 v_position;
attribute vec3 v_rgb;

varying vec3 color;

void main()
{
	gl_Position = vec4(v_position * vec2(2) - vec2(1), 0, 1);
	color = v_rgb;
}
