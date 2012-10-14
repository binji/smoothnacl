uniform mat4 u_MVP;
attribute vec3 a_color; 
attribute vec4 a_position; 
varying vec3 v_color;
void main() {
  gl_Position = u_MVP * a_position;
  v_color = a_color;
}
