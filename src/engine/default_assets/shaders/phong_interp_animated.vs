#define MAX_BONES 100

uniform mat4 projection;
uniform mat4 world2camera;
uniform mat4 object2world;
uniform mat4 object2world_it; // Transpose of inverse
uniform mat4 bonePoses[MAX_BONES];

#ifndef GL_ES
in vec3 in_Position;
in vec3 in_Normal;
in ivec2 in_SkinIndex;
in vec2 in_SkinWeight;

out vec3 worldPosition;
out vec3 normalAtFragment;

void main()
{
	mat4 bonePose = bonePoses[in_SkinIndex.x]*in_SkinWeight.x
		          + bonePoses[in_SkinIndex.y]*in_SkinWeight.y;
	vec4 worldPosition4f = object2world * bonePose * vec4(in_Position.xyz, 1.0);

	gl_Position = projection * world2camera * worldPosition4f;
	worldPosition = worldPosition4f.xyz;
	normalAtFragment = (object2world_it * vec4(in_Normal, 0.0)).xyz;
}

#else

attribute vec3 in_Position;
attribute vec3 in_Normal;
attribute vec2 in_SkinIndex;
attribute vec2 in_SkinWeight;

varying vec3 worldPosition;
varying vec3 normalAtFragment;

void main()
{
	int idxX = int(in_SkinIndex.x);
	int idxY = int(in_SkinIndex.y);
	mat4 bonePose = bonePoses[idxX]*in_SkinWeight.x
		          + bonePoses[idxY]*in_SkinWeight.y;

	vec4 worldPosition4f = object2world * bonePose * vec4(in_Position.xyz, 1.0);

	gl_Position = projection * world2camera * worldPosition4f;
	worldPosition = worldPosition4f.xyz;
	normalAtFragment = (object2world_it * vec4(in_Normal, 0.0)).xyz;
}
#endif
