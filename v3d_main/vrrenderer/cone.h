#ifndef CONE_H
#define CONE_H



#include "RenderableObject.h"

class Cone : public RenderableObject {
public:
    Cone(float baseRadius, float height, int segments=100);
    ~Cone();
    GLenum GetPrimitiveType() override;
    void updateSize(float baseRadius, float height) override;
protected:
    void fillVertexBuffer() override;
    void fillIndexBuffer() override;
private:
    float baseRadius, height;
    int segments;
};


#endif // CONE_H
