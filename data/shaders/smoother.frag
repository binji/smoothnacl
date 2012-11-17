precision mediump float;

const float pi = 3.141592653589793;

uniform float u_b1;
uniform float u_b2;
uniform float u_d1;
uniform float u_d2;
uniform float u_sn;
uniform float u_sm;
uniform int u_mode;
uniform int u_sigmode;
uniform int u_sigtype;
uniform int u_mixtype;
uniform float u_dt;
uniform sampler2D u_tex0;
uniform sampler2D u_tex1;
uniform sampler2D u_tex2;
varying vec2 v_texcoord0;
varying vec2 v_texcoord1;
varying vec2 v_texcoord2;


// All func_* functions below map |x| to the range [0, 1].
float func_hard(float x, float a) {
  return step(a, x);
}

float func_linear(float x, float a, float ea) {
  return clamp((x - a) / ea + 0.5, 0.0, 1.0);
}

float func_hermite(float x, float a, float ea) {
  return smoothstep(a - ea/2.0, a + ea/2.0, x);
}

float func_sin(float x, float a, float ea) {
  x = clamp(x, a - ea/2.0, a + ea/2.0);
  return sin(pi * (x - a) / ea) * 0.5 + 0.5;
}

float func_smooth(float x, float a, float ea) {
  return 1.0 / (1.0 + exp(-(x - a) * 4.0 / ea));
}

float sigmoid_ab(float x, float a, float b) {
  float fa, fb;
  if (u_sigtype == 0) {
    fa = func_hard(x, a);
    fb = func_hard(x, b);
  } else if (u_sigtype == 1) {
    fa = func_linear(x, a, u_sn);
    fb = func_linear(x, b, u_sn);
  } else if (u_sigtype == 2) {
    fa = func_hermite(x, a, u_sn);
    fb = func_hermite(x, b, u_sn);
  } else if (u_sigtype == 3) {
    fa = func_sin(x, a, u_sn);
    fb = func_sin(x, b, u_sn);
  } else if (u_sigtype == 4) {
    fa = func_smooth(x, a, u_sn);
    fb = func_smooth(x, b, u_sn);
  }

  return fa * (1.0 - fb);
}

float sigmoid_mix(float x, float y, float m) {
       if (u_mixtype == 0) return mix(x, y, func_hard(m, 0.5));
  else if (u_mixtype == 1) return mix(x, y, func_linear(m, 0.5, u_sm));
  else if (u_mixtype == 2) return mix(x, y, func_hermite(m, 0.5, u_sm));
  else if (u_mixtype == 3) return mix(x, y, func_sin(m, 0.5, u_sm));
  else if (u_mixtype == 4) return mix(x, y, func_smooth(m, 0.5, u_sm));
}

void main() {
  float n, m, f;

  n = texture2D(u_tex0, v_texcoord0.xy).r;
  m = texture2D(u_tex1, v_texcoord1.xy).r;

  if (u_sigmode == 0)
    f = mix(sigmoid_ab(n, u_b1, u_b2), sigmoid_ab(n, u_d1, u_d2), m);
  else if (u_sigmode == 1)
    f = sigmoid_mix(sigmoid_ab(n, u_b1, u_b2), sigmoid_ab(n, u_d1, u_d2), m);
  else if (u_sigmode == 2)
    f = sigmoid_ab(n, mix(u_b1, u_d1, m), mix(u_b2, u_d2, m));
  else  /* u_sigmode == 3 */
    f = sigmoid_ab(n, sigmoid_mix(u_b1, u_d1, m), sigmoid_mix(u_b2, u_d2, m));

  if (u_mode > 0) {
    float g = texture2D(u_tex2, v_texcoord2.xy).r;

         if (u_mode == 1) f = g + u_dt * (2.0 * f - 1.0);
    else if (u_mode == 2) f = g + u_dt * (f - g);
    else if (u_mode == 3) f = m + u_dt * (2.0 * f - 1.0);
    else if (u_mode == 4) f = m + u_dt * (f - m);
  }

  gl_FragColor.r = clamp(f, 0.0, 1.0);
  // Pepper GL requires Alpha to be set for RGBA render targets
  gl_FragColor.a = 1.0;
}
