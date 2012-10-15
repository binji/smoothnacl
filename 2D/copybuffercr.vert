precision mediump float;
uniform mat4 u_mat;
attribute vec2 a_texcoord0;
attribute vec2 a_texcoord1;
attribute vec4 a_position;
varying vec2 v_texcoord0;
varying vec2 v_texcoord1;

void main() {
  gl_Position = u_mat * a_position;
  v_texcoord0 = a_texcoord0;
  v_texcoord1 = a_texcoord1;
}
