//
// Created by yahor on 2.4.18.
//

#ifndef DEMO_RENDERCONTEXT_H
#define DEMO_RENDERCONTEXT_H


#include <SDL_opengl.h>

class RenderContext {

    GLuint _program;
public:
    RenderContext();

    RenderContext(GLuint p);


    ~RenderContext();


    void bindTexture(GLenum index, int texture);

    void bindTexture(GLenum index, int texture, std::string loc, int pos) const;

    void bindVertices(GLuint vbo, size_t size)const;

    void bindValue(const std::string& loc, const float f) const ;
};


#endif //DEMO_RENDERCONTEXT_H
