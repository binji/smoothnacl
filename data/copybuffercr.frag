// SmoothLife
//
// 2D copybuffer complex real

precision mediump float;

uniform sampler2D tex0;
uniform sampler2D tex1;
varying vec2 v_texcoord0;
varying vec2 v_texcoord1;

void main() {
  int a;

  a = int(v_texcoord1.x);
  if ((a/2)*2==a) {
    gl_FragColor.r = texture2D(tex0, v_texcoord0.xy).r;
  } else {
    gl_FragColor.r = texture2D(tex0, v_texcoord0.xy).g;
  }
  // Pepper GL requires Alpha to be set for RGBA render targets
  gl_FragColor.a = 1.0;
}
