#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 460 core

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

in vec2 v_TexCoord;

uniform sampler2D u_TextureDiffuse1;
uniform int u_EntityID;

void main()
{
	color = vec4(texture(u_TextureDiffuse1, v_TexCoord).xyz, 1.0f);
	color2 = u_EntityID;
}