uniform bool bDiffuse = false;
uniform sampler2D iDiffuse;
uniform bool bNormal = false;
uniform sampler2D iNormal;
uniform vec4 vecColor = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec3 vecCameraPosition;

uniform bool bLighted;
uniform vec3 vecSunlight;

uniform bool bRimLighting;

uniform float flAlpha;

in vec3 vecFragmentLocalPosition;
in vec3 vecFragmentGlobalPosition;
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
		// http://youtu.be/SZBkSYelJcg
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
		float flDiffuseTerm = dot(-vecSunlight, vecGlobalNormal);

		// If the surface normal is > 90 degrees away from the light then clamp the light value to 0.
		if (flDiffuseTerm < 0.0)
			flDiffuseTerm = 0.0;

		// Remap the light values so that they are a little softer
		flDiffuseTerm = RemapVal(flDiffuseTerm, 1.0, 0.0, 0.9, 0.2);

		vec3 vecToCamera = normalize(vecCameraPosition - vecFragmentGlobalPosition);
		vec3 vecHalf = normalize(vecToCamera - vecSunlight);

		// Generate the specular term http://youtu.be/hmKgNjQLm3A
		float flSpecularTerm = dot(vecGlobalNormal, vecHalf);

		// If the surface normal is > 90 degrees away from the light then clamp the light value to 0.
		if (flSpecularTerm < 0.0)
			flSpecularTerm = 0.0;

		float a = 15.0; // You can pull this from a specular map
		float c = 0.8;  // You can pull this from a gloss map

		// a is the "shininess" of the specularity
		// c is the brightness of the specular term
		flSpecularTerm = pow(flSpecularTerm, a);

		flLight = flDiffuseTerm + c * flSpecularTerm;
	}

	vec3 vecToCamera = vecCameraPosition - vecFragmentGlobalPosition;
	float flToCameraLength = length(vecCameraPosition - vecFragmentGlobalPosition);
	vec3 vecToCameraNormalized = vecToCamera/flToCameraLength;

	// Multiply that by the color to make a shadow
	vec4 vecDiffuse = vecColor * flLight;

	if (bRimLighting)
	{
		// Rim lighting. http://youtu.be/hB_F3KwMCkU
		float flRimLight = 1 - dot(vecToCameraNormalized, vecGlobalNormal);
		flRimLight = clamp(flRimLight - 0.5, 0.0, 1.0);

		if (gl_FragCoord.x < 400.0)
			vecDiffuse += flRimLight * vec4(0.4, 0.4, 1.0, 1.0);
	}

	// Add in a diffuse if there is one. http://youtu.be/aw6Vi-_hwy0
	if (bDiffuse)
		vecDiffuse *= texture(iDiffuse, vecFragmentTexCoord0);

	// Add in some fog. http://youtu.be/YpKVXNPOXg8
	float flDistance = flToCameraLength;

	float flFogMin = 10.0;
	float flFogMax = 50.0;
	float flFog = RemapValClamped(flDistance, flFogMin, flFogMax, 0.0, 1.0);

	vec4 vecFogColor = vec4(0.76, 0.8, 0.85, 1.0);
	vec4 vecFoggedDiffuse = vecDiffuse * (1-flFog) + vecFogColor * flFog;

	// Use that as our output color
	vecOutputColor = vecFoggedDiffuse;
	vecOutputColor.r = 1.0;

	if (vecDiffuse.a < 0.01)
		discard;
}
