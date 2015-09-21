#define MAX_LIGHTS 10

uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec4 lightColors[MAX_LIGHTS];


#ifndef GL_ES
precision highp float;

out vec4 fragColor;
in vec3 normalAtFragment;
in vec3 worldPosition;

void main()
{
	vec3 finalColor = vec3(0,0,0);
	const vec3 surfaceColor = vec3(1.0,0.5,0.25);
	vec3 normal = normalize(normalAtFragment);
	for(int i = 0; i < numLights && i < MAX_LIGHTS; ++i) {
		vec3 lightToFragmentDir = normalize(worldPosition-lightPositions[i]);
		float intensity = clamp(dot(-lightToFragmentDir, normal), 0, 1);
		finalColor += intensity * lightColors[i].w * lightColors[i].xyz * surfaceColor;
		//finalColor.x=lightToFragmentDir.y;
	}
	//finalColor.yz *= 0.0001;
	fragColor = vec4(finalColor,1.0);
}

#else
varying vec3 worldPosition;

void main( void )
{
	gl_FragColor = vec4(1.0,0.5,0.25,1.0);
}

#endif
