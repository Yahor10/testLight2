//
// Created by yahor on 2.4.18.
//

#include "LightSprite.h"
void LightSprite::Initialize() {
}

void LightSprite::LoadResources() {

}

void LightSprite::Update(float dt) {
}

void LightSprite::Dispose() {
    glDeleteTextures(1, &color);
    glDeleteTextures(1, &normal);
}

void LightSprite::Draw(const RenderContext& rc) {
//    rc.bindVertices(vbo, sizeof(vertices));
//    rc.bindValue("uPosition", this->m_position);

    rc.bindTexture(GL_TEXTURE0,color,"tSprite",0);
    rc.bindTexture(GL_TEXTURE1,normal,"tNormal",1);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//    rc.Draw(6); //2 tris
}

void LightSprite::DrawOcclusion(const RenderContext &rc) {
//    rc.bindVertices(vbo, sizeof(vertices));
//    rc.bindValue("uPosition", this->m_position);
//
    rc.bindTexture(GL_TEXTURE0,color,"tSprite",0);
    rc.bindTexture(GL_TEXTURE0,normal,"tNormal",1);
//
    rc.Draw(6); //2 tris
}

//void Sprite::DrawOverlayMask(const RenderContext& rc) {
//}

//The vertex array we're gonna draw.
//We're just drawing two tris so we can take care of other stuff in the vertex shader.
const Vector3 LightSprite::vertices[6] = {
        Vector3(0, 0, 1),
        Vector3(0, 1, 1),
        Vector3(1, 0, 1),
        Vector3(0, 1, 1),
        Vector3(1, 0, 1),
        Vector3(1, 1, 1)
};