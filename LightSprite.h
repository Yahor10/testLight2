//
// Created by yahor on 2.4.18.
//

#ifndef DEMO_LIGHTSPRITE_H
#define DEMO_LIGHTSPRITE_H


#include "IDisplayLight.h"

class LightSprite  : public IDisplayLight{
public:
    LightSprite(){

    }

    LightSprite(const int c, const int n)
            : color(c), normal(n) {
        m_position = Vector3(0, 0, 0.4);


    }

    virtual void Initialize();
    virtual void LoadResources();
    virtual void Update(float dt);
    virtual void Dispose();

    virtual void Draw(const RenderContext &render);
    virtual void DrawOcclusion(const RenderContext &render);

protected:
    //Just holds 00 01 10  01 10 11 vertices.  //Transformed by the vs.
    static const Vector3 vertices[6];

    //This gets set / unset for each sprite instance.
    //This isn't optimal but it's not worth it to reference-count a
    //shared vbo for each.
    GLuint vbo;

    GLuint color, normal; // textures
    //TODO: If we do rotations and stuff, do a Transform object. But not for the demo stuff.
};


#endif //DEMO_LIGHTSPRITE_H
