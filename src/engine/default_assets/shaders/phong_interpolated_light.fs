#define MAX_LIGHTS 10
varying vec3 worldPosition;

uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec4 lightColors[MAX_LIGHTS];
uniform uint numLights;

#ifndef GL_ES
precision highp float;
out vec4 fragColor;

void main()
{
	fragColor = vec4(1.0,0.5,0.25,1.0);
}

#else
in vec3 worldPosition;

void main( void )
{
	vec4 finalColor = vec4(0,0,0,0);
	const vec4 surfaceColor = vec4(1.0,0.5,0.25,1.0);
	for(int i = 0; i < numLights && i < MAX_LIGHTS; ++i) {
		float intensity = dot(-lightToFragmentDir, );
		finalColor += intensity * lightColors[i] * surfaceColor;
	}
	gl_FragColor = finalColor;
}

#endif
