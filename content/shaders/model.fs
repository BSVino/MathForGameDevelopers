uniform bool bDiffuse = false;
uniform sampler2D iDiffuse;
uniform bool bNormal = false;
uniform sampler2D iNormal;
uniform vec4 vecColor = vec4(1.0, 1.0, 1.0, 1.0);

uniform bool bLighted;
uniform vec3 vecSunlight;

uniform float flAlpha;

in vec3 vecFragmentPosition;
in vec2 vecFragmentTexCoord0;
in vec3 vecFragmentNormal;
in vec3 vecFragmentTangent;
in vec3 vecFragmentBitangent;
in vec3 vecFragmentColor;

uniform mat4 mView;
uniform mat4 mGlobal;

void main()
{
	// This 3x3 matrix should have the rotation components only. We need it to transform the fragment normals into world space.
	mat3 mGlobal3x3 = mat3(mGlobal);

	vec3 vecGlobalNormal;
	if (bNormal)
	{
		vec3 vecLocalNormal = normalize(texture(iNormal, vecFragmentTexCoord0).xyz * 2.0 - 1.0);

		// Since vecFragmentTexCoord0.y gets flipped in the vertex shader for texturing reasons,
		// we have to flip this normal again to get correct lighting.
		vecLocalNormal.y = -vecLocalNormal.y;

		// Create a matrix to transform the tangent from texture/triangle space to the object's local space.
		// http://youtu.be/TpQudLqiFfU
		mat3 mTBN = mat3(vecFragmentTangent, vecFragmentBitangent, vecFragmentNormal);
		vecGlobalNormal = normalize(mGlobal3x3*mTBN*vecLocalNormal);
	}
	else
	{
		// Transform the local space fragment normal into a global space vector and normalize it.
		// http://youtu.be/0zmLe4SssJc
		vecGlobalNormal = normalize(mGlobal3x3*vecFragmentNormal);
	}

	float flLight = 1;
	if (bLighted)
	{
		// Dot product of the sunlight vector and the normal vector of this surface.
		// http://youtu.be/0zmLe4SssJc
		float flLightDot = dot(-vecSunlight, vecGlobalNormal);

		// If the surface normal is > 90 degrees away from the light then clamp the light value to 0.
		if (flLightDot < 0)
			flLightDot = 0;

		// Remap the light values so that they are a little softer
		flLight = RemapVal(flLightDot, 1.0, 0.0, 0.9, 0.2);
	}

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
