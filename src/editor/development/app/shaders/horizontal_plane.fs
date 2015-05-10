precision highp float;
varying vec2 vUv;
uniform float nx;
uniform float ny;
varying vec3 worldPos;
void main(void) {
  const vec3 O = vec3(0,0,-1);

  vec3 P = vec3(0,nx,ny); P=normalize(P);
  vec3 Px = vec3(1,0,0); Px=normalize(Px);
  vec3 Py = vec3(0,ny,-nx); Py=normalize(Py);
  vec3 x = vec3(0, 0, 1.0+abs(nx)*30.0);
  float k = dot(P,x);

  float num = k-dot(P,worldPos);
  float den = dot(P,(worldPos-O));
  float transparency = 0.0;
  float alpha = num/den;
  if(alpha > -0.5) {
    vec3 v = (worldPos-O)*alpha + worldPos;
    vec3 vx = v-x;
    // Decompose the point x->v into a and b, where
    // x->b = a*Px + b*Py
    // and Px and Py are the orthonormal basis centered
    // at x and parallel to the plane
    float a = dot(vx, Px);
    float b = dot(vx, Py);

    // Make a and b periodic and shifted 0.5 so that
    // 0.5 is at the center of the lines
    a = mod(a+0.5, 1.0);
    b = mod(b+0.5, 1.0);

    // Transform the periodic function into something
    // that peaks at the lines
    a = clamp((50.0-abs(a*100.0-50.0))-49.9+alpha/2.0, 0.0, 1.0);
    b = clamp((50.0-abs(b*100.0-50.0))-49.9+alpha/2.0, 0.0, 1.0);

    // Fade with distance
    float ab=min(a+b,1.0)/max(alpha/2.0, 1.0);
    transparency=ab;
  } else {
    discard;
  }

  gl_FragColor = vec4(vec3(1.0), transparency);
}
