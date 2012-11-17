precision mediump float;
uniform sampler2D u_tex0;
varying vec2 v_texcoord0;

void main() {
  gl_FragColor.rgb = texture2D(u_tex0, v_texcoord0.xy).rrr;
  // Pepper GL requires Alpha to be set for RGBA render targets
  gl_FragColor.a = 1.0;
}
