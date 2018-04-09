//
// Created by yahor on 9.4.18.
//

#ifndef DEMO_LIGHTBACKGROUND_H
#define DEMO_LIGHTBACKGROUND_H


#include "LightSprite.h"

class LightBackground : public LightSprite {
public:
    LightBackground(const int c, const int n)
            : LightSprite(c, n) {}
    virtual void Draw(const RenderContext& render);
    virtual void DrawOcclusion(const RenderContext& render);
};


#endif //DEMO_LIGHTBACKGROUND_H
