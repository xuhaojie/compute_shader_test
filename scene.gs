#version 460 core

layout(points) in ;
layout(triangle_strip, max_vertices = 5) out; // ע���������

out vec3 color;

in VS_OUT {
    vec3 color;
} gs_in[];

void makeHouse(vec4 position)
{
	color = gs_in[0].color; 
	gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);    // 1:���� 
	EmitVertex();   
	gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0);    // 2:����
	EmitVertex();
	gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0);    // 3:����
	EmitVertex();
	gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0);    // 4:����
	EmitVertex();
	gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0);    // 5:����
	color = vec3(1.0, 1.0, 1.0); // ���췿���ǰ׶���Ч��
	EmitVertex();
	EndPrimitive();  
}

// ���������ʽ�����δ�
void main()
{
    gl_PointSize = gl_in[0].gl_PointSize;
	makeHouse(gl_in[0].gl_Position);
}