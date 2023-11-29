#pragma once

[[nodiscard]] inline ShaderProgramRef CreateNewMeshSceneShader()
{
	const char* vertexShaderText = R"(
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aTangents;
layout(location = 3) in vec4 aColor;
layout(location = 4) in vec2 aTexCoord;
layout(location = 5) in vec2 aTexCoord2;

uniform mat4 uWorld;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 fragmentNormal;
out vec4 fragmentTangents;
out vec4 fragmentColor;
out vec2 fragmentTexCoord;
out vec2 fragmentTexCoord2;

void main()
{
	gl_Position = uProjection * uView * uWorld * vec4(aPosition, 1.0);
	fragmentNormal = aNormal;
	fragmentTangents = aTangents;
	fragmentColor = aColor;
	fragmentTexCoord = aTexCoord;
	fragmentTexCoord2 = aTexCoord2;
}
)";

	const char* fragmentShaderText = R"(
in vec3 fragmentNormal;
in vec4 fragmentTangents;
in vec4 fragmentColor;
in vec2 fragmentTexCoord;
in vec2 fragmentTexCoord2;

out vec4 FinalFragmentColor;

uniform sampler2D DiffuseTexture;

void main()
{
	vec4 textureClr = texture(DiffuseTexture, fragmentTexCoord) * fragmentColor;
	if (textureClr.a < 0.02) discard;

	FinalFragmentColor = textureClr;
}
)";

	return GetRenderSystem().CreateShaderProgram({ vertexShaderText }, { fragmentShaderText });
}