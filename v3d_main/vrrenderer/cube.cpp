#include "cube.h"

Cube::Cube(float size) : size(size) {
    Init();
}

Cube::~Cube() {
    // Cube-specific cleanup, if necessary
}

GLenum Cube::GetPrimitiveType() {
    return GL_TRIANGLES;
}

void Cube::fillVertexBuffer() {
    vertices = {
        // 顶点数据 (包括顶点位置)
        glm::vec3(-size, -size, -size),
        glm::vec3( size, -size, -size),
        glm::vec3( size,  size, -size),
        glm::vec3(-size,  size, -size),
        glm::vec3(-size, -size,  size),
        glm::vec3( size, -size,  size),
        glm::vec3( size,  size,  size),
        glm::vec3(-size,  size,  size)
    };
}

void Cube::fillIndexBuffer() {
    indices = {
        0, 1, 2, 2, 3, 0, // 前面
        4, 5, 6, 6, 7, 4, // 后面
        0, 1, 5, 5, 4, 0, // 底面
        2, 3, 7, 7, 6, 2, // 顶面
        0, 3, 7, 7, 4, 0, // 左面
        1, 2, 6, 6, 5, 1  // 右面
    };
}
void Cube::updateSize(float size_n, float unused) {
    size = size_n;
    fillVertexBuffer();
    fillIndexBuffer();
    indices.clear();
    vertices.clear();
    Init();
}
