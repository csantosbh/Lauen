#ifndef GL_ES
in vec3 in_Position;

uniform mat4 projection;
uniform mat4 world2camera;
uniform mat4 object2world;

varying vec3 worldPosition;

void main()
{
	vec4 worldPosition4f = object2world * vec4(in_Position.xyz, 1.0);

	gl_Position = projection * world2camera * worldPosition4f;
	worldPosition = worldPosition4f.xyz;
}

#else

attribute vec3 in_Position;

uniform mat4 projection;
uniform mat4 world2camera;
uniform mat4 object2world;

out vec3 worldPosition;

void main()
{
	vec4 worldPosition4f = object2world * vec4(in_Position.xyz, 1.0);

	gl_Position = projection * world2camera * worldPosition4f;
	worldPosition = worldPosition4f.xyz;
}
#endif
