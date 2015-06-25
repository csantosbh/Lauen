#ifndef GL_ES
precision highp float;
out vec4 fragColor;

void main()
{
	fragColor = vec4(1.0,0.5,0.25,1.0);
}

#else

void main( void )
{
	gl_FragColor = vec4(1.0,0.5,0.25,1.0);
}

#endif
