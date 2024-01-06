// Renderer2D Sprite Shader

#type vertex
#version 330 core
 
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TextureCoord;
layout(location = 3) in float a_TextureIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_EntityID;

out vec4 v_Color;
out vec2 v_TextureCoord;
flat out float v_TextureIndex;
out float v_TilingFactor;	
flat out int v_EntityID;

uniform mat4 u_ViewProjection;

void main()
{
	v_Color = a_Color;
	v_TextureCoord = a_TextureCoord;
	v_TextureIndex = a_TextureIndex;
	v_TilingFactor = a_TilingFactor;
	v_EntityID = a_EntityID;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0f);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;
layout(location = 1) out int entityID;

in vec4 v_Color;
in vec2 v_TextureCoord;
flat in float v_TextureIndex;
in float v_TilingFactor;
flat in int v_EntityID;

uniform sampler2D u_Textures[32];

void main()
{
	vec4 texColor = v_Color;
	texColor *= texture(u_Textures[int(v_TextureIndex)], v_TextureCoord * v_TilingFactor);
	
	if(texColor.a == 0.0)
	{
		discard;
	}

	color = texColor;
	entityID = v_EntityID;
}