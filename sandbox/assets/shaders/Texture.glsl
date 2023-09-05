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
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Transform, 1.0);
	v_TextureCoord = a_TextureCoord;
}

#type fragment
#version 330 core

in vec2 v_TextureCoord;

out vec4 color;

uniform sampler2D u_Texture;

void main()
{
	color = texture2D(u_Texture, v_TextureCoord);
}
