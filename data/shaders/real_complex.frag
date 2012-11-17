precision mediump float;

uniform sampler2D u_tex0;
uniform sampler2D u_tex1;
varying vec2 v_texcoord0;
varying vec2 v_texcoord1;

void main() {
  gl_FragColor.r = texture2D(u_tex0, v_texcoord0.xy).r;
  gl_FragColor.g = texture2D(u_tex1, v_texcoord1.xy).r;
  // Pepper GL requires Alpha to be set for RGBA render targets
  gl_FragColor.a = 1.0;
}
