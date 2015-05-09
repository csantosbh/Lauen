precision highp float;
varying vec2 vUv;
uniform float color;
varying vec3 worldPos;
void main(void) {
  vec3 opticalCenter = vec3(0,0,-1);
  vec3 color = vec3(abs(worldPos.y));
  float alpha = 0.4;
  gl_FragColor = vec4(color, alpha);
}
