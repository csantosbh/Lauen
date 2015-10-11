uniform mat4 projection;
uniform mat4 world2camera;
uniform mat4 object2world;
uniform mat4 object2world_it; // Transpose of inverse

#ifndef GL_ES
in vec3 in_Position;
in vec3 in_Normal;
in ivec3 in_SkinIndex;
in vec3 in_SkinWeight;

out vec3 worldPosition;
out vec3 normalAtFragment;

void main()
{
	vec4 worldPosition4f = object2world * vec4(in_Position.xyz, 1.0);

	gl_Position = projection * world2camera * worldPosition4f;
	worldPosition = worldPosition4f.xyz;
	normalAtFragment = (object2world_it * vec4(in_Normal, 0.0)).xyz;
}

#else

attribute vec3 in_Position;
attribute vec3 in_Normal;
attribute ivec2 in_SkinIndex;
attribute vec2 in_SkinWeight;

varying vec3 worldPosition;
varying vec3 normalAtFragment;

void main()
{
	vec4 worldPosition4f = object2world * vec4(in_Position.xyz, 1.0);

	gl_Position = projection * world2camera * worldPosition4f;
	worldPosition = worldPosition4f.xyz;
	normalAtFragment = (object2world_it * vec4(in_Normal, 0.0)).xyz;
}
#endif
