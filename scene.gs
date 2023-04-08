#version 460 core

layout(points) in ;
layout(triangle_strip, max_vertices = 5) out; // 注意输出类型

out vec3 color;

in VS_OUT {
    vec3 color;
} gs_in[];

void makeHouse(vec4 position)
{
	color = gs_in[0].color; 
	gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);    // 1:左下 
	EmitVertex();   
	gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0);    // 2:右下
	EmitVertex();
	gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0);    // 3:左上
	EmitVertex();
	gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0);    // 4:右上
	EmitVertex();
	gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0);    // 5:顶部
	color = vec3(1.0, 1.0, 1.0); // 制造房屋是白顶的效果
	EmitVertex();
	EndPrimitive();  
}

// 输出房子样式三角形带
void main()
{
    gl_PointSize = gl_in[0].gl_PointSize;
	makeHouse(gl_in[0].gl_Position);
}