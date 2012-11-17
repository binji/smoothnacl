precision mediump float;

uniform int u_dim;
uniform int u_tang;
uniform float u_tangsc;
uniform sampler2D u_tex0;
uniform sampler2D u_tex1;
varying vec2 v_texcoord0;
varying vec2 v_texcoord1;

vec2 cmul(vec2 a, vec2 b) {
  return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

void main() {
  vec2 a, b;
  vec2 v;
  vec4 p;

  v = v_texcoord0.xy;

  if (u_dim == 1) {
    p = texture2D(u_tex1, vec2(v_texcoord1.x, 0)).rgba;
    a = texture2D(u_tex0, vec2(p.r, v.y)).rg;
    b = texture2D(u_tex0, vec2(p.g, v.y)).rg;
    if (u_tang == 1) {
      b.y = -b.y;
      gl_FragColor.rg = (a+b + cmul(a - b, p.ba)) * u_tangsc;
    } else {
      gl_FragColor.r = (a.r + p.b * b.r - p.a * b.g) * (1.0 / sqrt(2.0));
      gl_FragColor.g = (a.g + p.b * b.g + p.a * b.r) * (1.0 / sqrt(2.0));
    }
  } else {
    p = texture2D(u_tex1, vec2(v_texcoord1.y, 0)).rgba;
    a = texture2D(u_tex0, vec2(v.x, p.r)).rg;
    b = texture2D(u_tex0, vec2(v.x, p.g)).rg;
    gl_FragColor.rg = (a + cmul(p.ba,b)) * (1.0 / sqrt(2.0));
  }
  // Pepper GL requires Alpha to be set for RGBA render targets
  gl_FragColor.a = 1.0;
}
