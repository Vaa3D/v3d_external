#include"shader_m.h"
#include"GL/glew.h"
#include"GL/glut.h"

void _initialize(Shader* testshader, GLuint& testVAO,GLuint& testVBO ){
	cout << "run in initializeGL" << endl;
	//initializeOpenGLFunctions();
	glewInit();
	int test;
	cin >> test;
	//backfaceShader = new Shader(string("shader/backface.vert").c_str(), string("shader/backface.frag").c_str());
	//QString qappDirPath = QCoreApplication::applicationDirPath();
	//testshader = new Shader(string(qappDirPath.toStdString() + "shader/3.3.shader.vs").c_str(), string(qappDirPath.toStdString() + "shader/3.3.shader.frag").c_str());
	testshader = new Shader(string("C:/Users/57610/Documents/research/vaa3d_compile/v3d_external/v3d_main/v3d/release/3.3.shader.vs").c_str(), string("C:/Users/57610/Documents/research/vaa3d_compile/v3d_external/v3d_main/v3d/release/3.3.shader.frag").c_str()); // you can name your shader files however you like
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float test_vertices[] = {
		// positions         // colors 
		0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  // bottom left
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f   // top 
	};


	glGenVertexArrays(1, &testVAO);
	glGenBuffers(1, &testVBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(testVAO);

	glBindBuffer(GL_ARRAY_BUFFER, testVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(test_vertices), test_vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

}
void _paintGL(Shader* testshader,GLuint& testVAO){


	glClear(GL_COLOR_BUFFER_BIT);

	// draw our first triangle
	glUseProgram(testshader->ID);
	glBindVertexArray(testVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	//glDrawArrays(GL_TRIANGLES, 0, 6);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	// glBindVertexArray(0); // no need to unbind it every time 

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	// -------------------------------------------------------------------------------
	//glutSwapBuffers();
}