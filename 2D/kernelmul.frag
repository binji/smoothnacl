// SmoothLife
//
// 2D kernelmul

precision mediump float;

uniform float sc;
uniform sampler2D tex0;
uniform sampler2D tex1;
varying vec2 v_texcoord0;
varying vec2 v_texcoord1;

void main() {
  vec2 a, b;

  a = texture2D(tex0, v_texcoord0.xy).rg;
  b = texture2D(tex1, v_texcoord1.xy).rg*sc;
  gl_FragColor.r = a.r*b.r - a.g*b.g;
  gl_FragColor.g = a.r*b.g + a.g*b.r;
}
