precision mediump float;

uniform float u_scale;
uniform sampler2D u_tex0;
uniform sampler2D u_tex1;
varying vec2 v_texcoord0;
varying vec2 v_texcoord1;

void main() {
  vec2 a, b;

  a = texture2D(u_tex0, v_texcoord0.xy).rg;
  b = texture2D(u_tex1, v_texcoord1.xy).rg * u_scale;
  gl_FragColor.r = a.r * b.r - a.g * b.g;
  gl_FragColor.g = a.r * b.g + a.g * b.r;
  // Pepper GL requires Alpha to be set for RGBA render targets
  gl_FragColor.a = 1.0;
}
