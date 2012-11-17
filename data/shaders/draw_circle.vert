precision mediump float;

uniform mat4 u_mat;
uniform vec2 u_pos;
uniform float u_radius;
attribute vec4 a_position;  // x, y in range [0, 1].

void main() {
  vec2 translate = u_pos - vec2(u_radius, u_radius);
  float scale = 2.0 * u_radius;
  gl_Position = u_mat * vec4(a_position.xy * scale + translate, 0, 1);
}
