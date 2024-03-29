// Renderer2D Text Shader

#type vertex
#version 330 core
 
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_TextColor;
layout(location = 2) in vec4 a_BGColor;
layout(location = 3) in vec2 a_TextureCoord;
layout(location = 4) in int a_EntityID;

out vec4 v_TextColor;
out vec4 v_BGColor;
out vec2 v_TextureCoord;
flat out int v_EntityID;

uniform mat4 u_ViewProjection;

void main()
{
	v_TextColor = a_TextColor;
	v_BGColor = a_BGColor;
	v_TextureCoord = a_TextureCoord;
	v_EntityID = a_EntityID;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0f);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;
layout(location = 1) out int entityID;

in vec4 v_TextColor;
in vec4 v_BGColor;
in vec2 v_TextureCoord;
flat in int v_EntityID;

uniform sampler2D u_TextAtlas;

float screenPxRange() 
{
	const float pxRange = 2.0; // set to distance field's pixel range
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_TextAtlas, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(v_TextureCoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) 
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
	vec3 msd = texture(u_TextAtlas, v_TextureCoord).rgb;
	float sd = median(msd.r, msd.g, msd.b);
	float screenPxDistance = screenPxRange() * (sd - 0.5);
	float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
	if (opacity == 0.0)
		discard;
	color = mix(v_BGColor, v_TextColor, opacity);
	if (color.a == 0.0)
		discard;

	entityID = v_EntityID;
}