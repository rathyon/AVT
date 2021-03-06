#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria
in vec4 texCoord;

out Data {
	vec3 normal;
	vec3 eye;
	vec3 position;
	vec2 tex_coord;
} DataOut;

out float fogFactor;

void main () {

	vec4 pos = m_viewModel * position;

	DataOut.normal = normalize(m_normal * normal.xyz);

	DataOut.position = vec3(pos);

	DataOut.eye = vec3(-pos);

	DataOut.tex_coord = texCoord.st;

	gl_Position = m_pvm * position;

	//fogFactor = clamp((80 - length(DataOut.eye))/ (80 - 20), 0, 1); //GL_LINEAR formula

	fogFactor = exp((-0.01f * length(DataOut.eye)));  // GL_EXP2 formula
}