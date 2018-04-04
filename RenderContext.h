//
// Created by yahor on 2.4.18.
//

#ifndef DEMO_RENDERCONTEXT_H
#define DEMO_RENDERCONTEXT_H


#include <SDL_opengl.h>
#include <oxygine/math/Vector2.h>
#include <oxygine/math/Vector3.h>

using namespace oxygine;

struct myVertex {
    Vector3 pos;
    Vector2 uv;
};

class RenderContext {

    GLuint _program;
public:
    RenderContext();

    RenderContext(GLuint p);


    ~RenderContext();


    void useProgram(GLuint p){
        _program = p;
        oxglUseProgram(_program);
    }

    void bindTexture(GLenum index, int texture);

    void bindTexture(GLenum index, int texture, std::string loc, int pos) const;

    void bindVertices(GLuint vbo, size_t size)const;

    void bindPosVertices(int vbo, size_t size,myVertex* arr)const;

    void bindUVVertices(int vbo, size_t size,myVertex* arr)const;

    void bindQuadVertices(int vbo,size_t size)const;


    void bindValue(const std::string& loc, const float f) const ;

    void bindValue(const std::string&& loc, const float f) const;
    void bindValue(const std::string&& loc, const bool b) const;
    void bindValue(const std::string&& loc, const Vector2& v) const;
    void bindValue(const std::string&& loc, const Vector3& v) const;

    //void RenderContext::bindValue(const string& loc, const glm::mat3x3& m) const {// TODO set matrix

    void Draw(const size_t count) const;
};


#endif //DEMO_RENDERCONTEXT_H
