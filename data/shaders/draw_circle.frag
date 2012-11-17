precision mediump float;

uniform sampler2D u_tex0;
uniform vec2 u_window_size;
uniform vec2 u_pos;
uniform float u_radius;

void main() {
  float circle_color = float(length(gl_FragCoord.xy - u_pos) < u_radius);
  float bg_color = texture2D(u_tex0, gl_FragCoord.xy / u_window_size).r;
  gl_FragColor.r = max(circle_color, bg_color);
}
