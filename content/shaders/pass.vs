uniform vec4 vecColor;

in vec3 vecPosition;
in vec3 vecNormal;
in vec3 vecTangent;
in vec3 vecBitangent;
in vec2 vecTexCoord0;
in vec3 vecVertexColor;

out vec3 vecFragmentPosition;
out vec3 vecFragmentNormal;
out vec3 vecFragmentTangent;
out vec3 vecFragmentBitangent;
out vec2 vecFragmentTexCoord0;
out vec3 vecFragmentColor;

void main()
{
	gl_Position = mProjection * mView * mGlobal * vec4(vecPosition, 1.0);

	vecFragmentPosition = vecPosition;
	vecFragmentNormal = vecNormal;
	vecFragmentTangent = vecTangent;
	vecFragmentBitangent = vecBitangent;
	vecFragmentTexCoord0 = vec2(vecTexCoord0.x, 1-vecTexCoord0.y);
	vecFragmentColor = vecVertexColor;
}
