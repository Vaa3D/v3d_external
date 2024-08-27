#define _USE_MATH_DEFINES
#include <cmath>
#include "Cone.h"
#include <glm/glm.hpp>
#include <vector>

Cone::Cone(float baseRadius, float height, int segments)
    : baseRadius(baseRadius), height(height), segments(segments) {
    Init();
}

Cone::~Cone() {
    // Cone-specific cleanup, if necessary
}

GLenum Cone::GetPrimitiveType() {
    return GL_TRIANGLE_STRIP;
}

void Cone::fillVertexBuffer() {
    // 顶部顶点
    vertices.push_back(glm::vec3(0.0f, height, 0.0f)); // 顶点

    // 底部顶点
    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / (float)segments * 2.0f * M_PI;
        float x = baseRadius * cos(angle);
        float z = baseRadius * sin(angle);
        vertices.push_back(glm::vec3(x, 0.0f, z));
    }
}

void Cone::fillIndexBuffer() {
    // 侧面索引
    for (int i = 1; i <= segments; ++i) {
        indices.push_back(0); // 顶点
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    // 底部索引
    for (int i = 1; i < segments; ++i) {
        indices.push_back(1); // 第一个底部顶点
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }
}
void Cone::updateSize(float newBaseRadius, float newHeight) {
    baseRadius = newBaseRadius;
    height = newHeight;
    segments = 36;
    indices.clear();
    vertices.clear();
    Init();
}
