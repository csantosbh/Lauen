#ifndef GL_ES
in vec3 in_Position;

uniform mat4 projection;
uniform mat4 world2camera;

void main()
{
	gl_Position = projection * world2camera * vec4(in_Position.xyz, 1.0);
}

#else

attribute vec3 in_Position;

uniform mat4 projection;
uniform mat4 world2camera;

void main()
{
	gl_Position = projection * world2camera * vec4(in_Position.xyz, 1.0);
}
#endif
