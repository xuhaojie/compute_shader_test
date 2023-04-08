// 引入GLEW库 定义静态链接
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "computer_shader.h"

const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;

#define USE_READ_BUFFER 0
#define OFF_SCREEN_MODE 0

#define EXECUTE_DRAW 1

#define EXECUTE_COMPUTE 1

#define TEXTURE_WIDTH	16    //data block width
#define TEXTURE_HEIGHT	16    //data block height

const unsigned unWidth = (unsigned)TEXTURE_WIDTH;
const unsigned unHeight = (unsigned)TEXTURE_HEIGHT;
const unsigned unSize = unWidth * unHeight;

#if(EXECUTE_COMPUTE)
void transferToTexture(float* data, GLuint texID) {
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, unWidth, unHeight, GL_RGBA, GL_FLOAT, data);
}

// 读回数据
void GetDataFromTexture(float* data) {
#if(OFF_SCREEN_MODE && USE_READ_BUFFER) // 只有离屏幕式可以用
	glReadBuffer(GL_COLOR_ATTACHMENT2);
	glReadPixels(0, 0, unWidth, unHeight, GL_RGBA, GL_FLOAT, data);
#else
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data);
#endif
}

GLuint createTexture(int tex_w, int tex_h) {
	GLuint tex_output = 0;
	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
//	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	return tex_output;
}

void performCompute(const GLuint glslProgram,const GLuint inputTexID, const GLuint outputTexID, GLfloat v[4]) {
	// enable GLSL program
	glUseProgram(glslProgram);
	GLuint location = glGetUniformLocation(glslProgram, "v");
	glUniform1fv(location, 4, v);
	// Synchronize for the timing reason.
	glFinish();
	glBindImageTexture(0, inputTexID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, outputTexID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glFinish();
}
#endif //#if(COMPUTE)

// 本例运行环境为ubuntu（非虚拟机，虚拟机一运行compute shader相关的代码就报 exit code 139），需要先安装好OpenGL环境，GLUT和GLEW。
// 在这个例子中，会先产生一个 1024 个数据，并将这些数据赋予一个 16 x 16 x 4 的image2D，
// 然后通过计算着色器对这个image2D的每个坐标点的4个通道分别加上0、1、2、3，最后将计算结果读取出来。
int main(int argc, char** argv)
{
	// 初始化glfw库
	if (!glfwInit())	
	{
		std::cout << "Error::GLFW could not initialize GLFW!" << std::endl;
		return -1;
	}

	// 开启OpenGL 4.6 core profile
	std::cout << "Start OpenGL core profile version 4.6" << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// 创建窗口
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Demo of geometry shader", NULL, NULL);
	if (nullptr == window)
	{
		printf("Error::GLFW could not create winddow!\n");
		glfwTerminate();
		std::system("pause");
		return -1;
	}

	// 创建的窗口的context指定为当前context
	glfwMakeContextCurrent(window);

	// 让glew获取所有拓展函数
	glewExperimental = GL_TRUE;

	// 初始化GLEW 获取OpenGL函数
	GLenum status = glewInit();
	if (status != GLEW_OK)
	{
		printf("GLEW glew version:%s, error string: %s\n", glewGetString(GLEW_VERSION), glewGetErrorString(status));
		glfwTerminate();
		std::system("pause");
		return -1;
	}

	int work_group_count[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_group_count[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_group_count[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_group_count[2]);
	printf("max global (total) work group counts x:%i y:%i z:%i\n", work_group_count[0], work_group_count[1], work_group_count[2]);

	int work_group_size[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_group_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_group_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_group_size[2]);

	printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n", work_group_size[0], work_group_size[1], work_group_size[2]);

	int work_group_invocation;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_group_invocation);
	printf("max local work group invocations %i\n", work_group_invocation);

	// 设置视口参数
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glEnable(GL_PROGRAM_POINT_SIZE); // 启用在顶点着色器中指定点的大小

#if(OFF_SCREEN_MODE)
	// 离屏渲染
	GLuint frame_buffer;
	// create FBO (off-screen framebuffer)
	glGenFramebuffers(1, &frame_buffer);
	// bind offscreen framebuffer (that is, skip the window-specific render target)
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
#endif // #if(OFF_SCREEN)
	
#if(EXECUTE_DRAW)
	//Section1 顶点属性数据
// 指定顶点属性数据 顶点位置

	float points[] = {
		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // 左上
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // 右上
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // 右下
		-0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // 左下
	};


	// 创建VBO
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	// 创建VAO
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	// 顶点位置属性
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,	5 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 顶点颜色属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,	5 * sizeof(GL_FLOAT), (GLvoid*)(2 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);

#endif // #if(DRAW)

	// Section3 准备着色器程序
	Shader shader("scene.vertex", "scene.frag", "scene.gs"); // 添加了几何着色器
	
#if(EXECUTE_COMPUTE)
	ComputeShader computeShader("scene.cs"); // 添加了几何着色器

	GLuint outputTexID = createTexture(unWidth, unHeight);
	

	GLuint intermediateTexID = createTexture(unWidth, unHeight);

	GLuint inputTexID = createTexture(unWidth, unHeight);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, inputTexID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, intermediateTexID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, outputTexID, 0);

	// set texenv mode
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// 创建测试数据
	const unsigned unNoData = unSize * sizeof(GLfloat);        //total number of Data

	float input_data[unNoData];
	for (int i = 0; i < unNoData; i++) input_data[i] = i * 0.001f;
	
	float output_data[unNoData];
	for (int i = 0; i < unNoData; i++) output_data[i] = 0.0f;

	GLfloat v[4];  // 传入compute shader中
	for (int i = 0; i < 4; i++) v[i] = i;

	transferToTexture(input_data, inputTexID);

	computeShader.use();

	performCompute(computeShader.programId, inputTexID, intermediateTexID, v);

	performCompute(computeShader.programId, intermediateTexID, outputTexID, v);

	// get GPU results
	glBindTexture(GL_TEXTURE_2D, outputTexID);
	GetDataFromTexture(output_data);

	for (int i = 0; i < unNoData; i++) 
	{
		printf("input:%f  output:%f\n", input_data[i], output_data[i]);
	}

#endif //#if(COMPUTE)

#if(EXECUTE_DRAW)
	// 开始游戏主循环
	while (!glfwWindowShouldClose(window))
	{
		// 设置colorBuffer颜色
		glClearColor(0.18f, 0.04f, 0.14f, 1.0f);
		// 清除colorBuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 先绘制场景 演示几何着色器时 暂不使用MVP矩阵了
		glfwPollEvents(); // 处理例如鼠标 键盘等事件

		shader.use();
		glBindVertexArray(vao);
		glDrawArrays(GL_POINTS, 0, 4);
		glBindVertexArray(0);
		glUseProgram(0);

		glfwSwapBuffers(window); // 交换缓存
	}

	// 释放资源
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
#endif // #if(DRAW)
	glfwTerminate();
	return 0;
}