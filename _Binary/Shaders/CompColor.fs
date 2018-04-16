#version 330

in vec3 Color;

uniform vec3 SolidColor = vec3(-1,-1,-1);
uniform int CompColor = -1;

out vec4 Fragment;

void main()
{
	Fragment = vec4(Color,1);

	if(SolidColor.r != -1.0 && SolidColor.g != -1.0 && SolidColor.b != -1.0)
		Fragment = vec4(SolidColor, 1);
	if(CompColor != -1)
	{
		vec4 newCol = Fragment;
		newCol.r = 1.0 - newCol.r;
		newCol.g = 1.0 - newCol.g;
		newCol.b = 1.0 - newCol.b;
		Fragment = newCol;
	}

	return;
}
