#define MAX_LIGHTS 10
precision highp float;

uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec4 lightColors[MAX_LIGHTS];

const vec3 surfaceColor = vec3(1.0,0.5,0.25);

#ifndef GL_ES
out vec4 fragColor;
in vec3 normalAtFragment;
in vec3 worldPosition;

void main()
{
	vec3 finalColor = vec3(0,0,0);
	vec3 normal = normalize(normalAtFragment);
	for(int i = 0; i < numLights && i < MAX_LIGHTS; ++i) {
		vec3 lightToFragmentDir = normalize(worldPosition-lightPositions[i]);
		float intensity = clamp(dot(-lightToFragmentDir, normal), 0, 1);
		finalColor += intensity * lightColors[i].w * lightColors[i].xyz * surfaceColor;
	}
	fragColor = vec4(finalColor,1.0);
}

#else
varying vec3 normalAtFragment;
varying vec3 worldPosition;

void main( void )
{
	vec3 finalColor = vec3(0,0,0);
	vec3 normal = normalize(normalAtFragment);
	for(int i = 0; i < MAX_LIGHTS; ++i) {
		vec3 lightToFragmentDir = normalize(worldPosition-lightPositions[i]);
		float intensity = clamp(dot(-lightToFragmentDir, normal), 0.0, 1.0);
		finalColor += intensity * lightColors[i].w * lightColors[i].xyz * surfaceColor;
	}
	gl_FragColor = vec4(finalColor,1.0);
}

#endif
