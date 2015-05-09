varying vec2 vUv;
varying vec3 worldPos;
void main() {
  vUv = uv;
  gl_Position = vec4(position, 1.0 );
  worldPos = position;
}
