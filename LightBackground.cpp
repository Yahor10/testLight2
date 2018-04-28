//
// Created by yahor on 9.4.18.
//

#include "LightBackground.h"

void LightBackground::Draw(const RenderContext &rc) {
    rc.bindTexture("tSprite", 0, this->color);
    rc.bindTexture("tNormal", 1, this->normal);
    rc.bindVertices(vbo, sizeof(vertices));
    rc.bindValue("uPosition", m_position);
    rc.Draw(6);
}

void LightBackground::Draw(const RenderContext &rc, int c, int n) {

    rc.bindTexture("tSprite", 0, c);
    rc.bindTexture("tNormal", 1, n);
    rc.bindVertices(vbo, sizeof(vertices));
    rc.bindValue("uPosition", m_position);
    rc.Draw(6);
}


//Background doesn't cast shadows.
void LightBackground::DrawOcclusion(const RenderContext &rc) { }
