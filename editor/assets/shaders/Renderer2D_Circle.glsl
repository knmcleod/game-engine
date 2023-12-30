// Renderer2D Circle Shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_GlobalPosition;
layout(location = 1) in vec3 a_LocalPosition;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_Radius;
layout(location = 4) in float a_Thickness;
layout(location = 5) in float a_Fade;
layout(location = 6) in int a_EntityID;

out vec3 v_LocalPosition;
out vec4 v_Color;
out float v_Radius;
out float v_Thickness;
out float v_Fade;
flat out int v_EntityID;

uniform mat4 u_ViewProjection;

void main()
{
	v_LocalPosition = a_LocalPosition;
	v_Color = a_Color;
	v_Radius = a_Radius;
	v_Thickness = a_Thickness;
	v_Fade = a_Fade;
	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_GlobalPosition, 1.0f);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;
layout(location = 1) out int entityID;

in vec3 v_LocalPosition;
in vec4 v_Color;
in float v_Radius;	
in float v_Thickness;	
in float v_Fade;
flat in int v_EntityID;

void main()
{
	float d = length(vec2(v_LocalPosition.x, v_LocalPosition.y));

	float circle = (1.0 - smoothstep(v_Thickness - v_Radius, v_Radius, d)) * (smoothstep(v_Fade - v_Radius, v_Radius, d));
	if(circle == 0)
	{
		discard;
	}
	
	color = vec4(v_Color.rgb, circle);

	entityID = v_EntityID;
}