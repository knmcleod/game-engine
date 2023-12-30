// Renderer2D Quad Shader

#type vertex
#version 330 core
 
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 5) in int a_EntityID;

out vec4 v_Color;
flat out int v_EntityID;

uniform mat4 u_ViewProjection;

void main()
{
	v_Color = a_Color;
	v_EntityID = a_EntityID;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0f);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;
layout(location = 1) out int entityID;

in vec4 v_Color;
flat in int v_EntityID;


void main()
{
	color = v_Color;
	entityID = v_EntityID;
}