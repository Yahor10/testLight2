//
// Created by yahor on 2.4.18.
//

#include <oxygine/core/gl/oxgl.h>
#include "RenderContext.h"



RenderContext::RenderContext() { }

RenderContext::~RenderContext() {
    oxglDeleteProgram(_program);
}

RenderContext::RenderContext(GLuint p) {
    _program = p;
    oxglUseProgram(_program);
}


void RenderContext::bindVertices(GLuint vbo, size_t size) const {// base aVertextValue
    oxglBindBuffer(GL_ARRAY_BUFFER, vbo);
    GLuint location = oxglGetAttribLocation(_program, "aVertex");
    oxglEnableVertexAttribArray(location);
    oxglVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

void RenderContext::bindTexture(GLenum index, int texture) {
    oxglActiveTexture(index);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void RenderContext::bindTexture(GLenum index, int texture, std::string loc, int pos) {
    int tex_a_location = oxglGetUniformLocation(_program, loc.c_str());
    oxglUniform1i(tex_a_location, pos);

    oxglActiveTexture(index);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void RenderContext::bindValue(const std::string& loc, const float f) const{
    GLint ul = oxglGetUniformLocation(_program, loc.c_str());
    oxglUniform1f(ul, f);
}
