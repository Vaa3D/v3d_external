#pragma once

#include <vector>
// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

class RenderableObject
{
public:
	RenderableObject();
	~RenderableObject();

	virtual GLenum GetPrimitiveType() = 0;
	virtual void fillVertexBuffer() = 0;
	virtual void fillIndexBuffer() = 0;

	void Init();
	void Render();

    virtual void updateSize(float size1, float size2 = 0) = 0; // 纯虚函数
    virtual void setColor(const glm::vec3& color);
    void setVisible(bool visible);


    bool isVisible() const;
	/*  Mesh Data  */
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<GLuint> indices;
    glm::vec3 color;
protected:
	GLuint vaoID;
	GLuint vboVerticesID;
	GLuint vboNormalsID;
	GLuint vboIndicesID;

    bool visible;
	GLenum primType;
};

