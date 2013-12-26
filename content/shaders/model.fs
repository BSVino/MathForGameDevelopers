uniform bool bDiffuse = false;
uniform sampler2D iDiffuse;
uniform vec4 vecColor = vec4(1.0, 1.0, 1.0, 1.0);

uniform vec3 vecSunlight;

uniform float flAlpha;

in vec3 vecFragmentPosition;
in vec2 vecFragmentTexCoord0;
in vec3 vecFragmentNormal;
in vec3 vecFragmentColor;

uniform mat4 mView;
uniform mat4 mGlobal;

void main()
{
	// This 3x3 matrix should have the rotation components only. We need it to transform the fragment normals into world space.
	mat3 mGlobal3x3 = mat3(mGlobal);

	// Transform the local space fragment normal into a global space vector and normalize it.
	// http://youtu.be/0zmLe4SssJc
	vec3 vecGlobalNormal = normalize(mGlobal3x3*vecFragmentNormal);

	// Dot product of the sunlight vector and the normal vector of this surface.
	// http://youtu.be/0zmLe4SssJc
	float flLightDot = dot(-vecSunlight, vecGlobalNormal);

	// Remap the light values so that they are a little softer
	float flLight = RemapVal(flLightDot, 1.0, 0.0, 0.9, 0.4);

	// Multiply that by the color to make a shadow
	vec4 vecDiffuse = vecColor * flLight;

	// Add in a diffuse if there is one. http://youtu.be/aw6Vi-_hwy0
	if (bDiffuse)
		vecDiffuse *= texture(iDiffuse, vecFragmentTexCoord0);

	// Use that as our output color
	vecOutputColor = vecDiffuse;

	if (vecDiffuse.a < 0.01)
		discard;
}
