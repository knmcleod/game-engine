// Basic Texture Shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Transform;
layout(location = 2) in vec2 a_TextureCoord;

out vec2 v_TextureCoord;
			
uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

void main()
{
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Transform, 1.0f);
	v_TextureCoord = a_TextureCoord;
}

#type fragment
#version 330 core

out vec4 color;

in vec2 v_TextureCoord;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main()
{
	color = texture(u_Texture, v_TextureCoord) * u_Color;
}
