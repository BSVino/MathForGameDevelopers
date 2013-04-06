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
	// Dot product of the sunlight vector and the normal vector of this surface
	float flLightDot = dot(vecSunlight, normalize(vecFragmentNormal));

	// Remap the light values so that the negative result becomes positive
	float flLight = RemapVal(flLightDot, -1.0, 0.0, 0.9, 0.4);

	// Multiply that by the color to make a shadow
	vec4 vecDiffuse = vecColor * flLight;

	// Add in a diffuse if there is one
	if (bDiffuse)
		vecDiffuse *= texture(iDiffuse, vecFragmentTexCoord0);

	// Use that as our output color
	vecOutputColor = vecDiffuse;
	vecOutputColor.a *= flAlpha;

	if (vecDiffuse.a < 0.01)
		discard;
}
