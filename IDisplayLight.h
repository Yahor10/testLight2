//
// Created by yahor on 2.4.18.
//

#ifndef DEMO_IDISPLAYLIGHT_H
#define DEMO_IDISPLAYLIGHT_H

#include "oxygine-framework.h"
#include "RenderContext.h"

using namespace oxygine;

class IDisplayLight {
public:
    virtual void Initialize() = 0;

    virtual void LoadResources() = 0;

    virtual void Update(float dt) = 0;

    virtual void Dispose() = 0;

    //position x,y and <xy>
    float x() {
        return m_position.x;
    }

    void x(float value) {
        m_position.x = value;
    }

    float y() {
        return m_position.y;
    }

    void y(float value) {
        m_position.y = value;
    }

    float z() {
        return m_position.z;
    }

    void z(float value) {
        m_position.z = value;
    }

    Vector2 position() const {
        return Vector2(m_position.x, m_position.y);
    }

    void position(const Vector2 &p) {
        m_position = Vector3(p.x, p.y, m_position.z);
    }

    virtual void Draw(const RenderContext &render) = 0;

    virtual void DrawOcclusion(const RenderContext &render) = 0;

protected:
    Vector3 m_position;
};


#endif //DEMO_IDISPLAYLIGHT_H
