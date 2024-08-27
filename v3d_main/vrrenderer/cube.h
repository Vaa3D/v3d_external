#ifndef CUBE_H
#define CUBE_H


#include "RenderableObject.h"

class Cube : public RenderableObject {
public:
    Cube(float size);
    ~Cube();
    GLenum GetPrimitiveType() override;
    void updateSize(float size, float unused = 0) override;
protected:
    void fillVertexBuffer() override;
    void fillIndexBuffer() override;
private:
    float size;
};
#endif // CUBE_H
