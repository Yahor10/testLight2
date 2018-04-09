//
// Created by yahor on 2.4.18.
//

#include <oxygine/core/gl/oxgl.h>
#include "RenderContext.h"


RenderContext::RenderContext() { }

RenderContext::~    RenderContext() {
    oxglDeleteProgram(_program);
}

RenderContext::RenderContext(GLuint p) {
    _program = p;
}

void RenderContext::bindVertices(GLuint vbo, size_t size) const {// base aVertextValue
    oxglBindBuffer(GL_ARRAY_BUFFER, vbo);
    GLuint location = oxglGetAttribLocation(_program, "aVertex");
    oxglEnableVertexAttribArray(location);
    oxglVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
}

void RenderContext::bindPosVertices(int pos, size_t size,myVertex* vector3) const {// base aVertextValue
    oxglEnableVertexAttribArray(pos);
    oxglVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, size, vector3);
}

void RenderContext::bindUVVertices(int pos, size_t size,myVertex* vector3) const {// base aVertextValue
    oxglEnableVertexAttribArray(pos);
    oxglVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, size, (float*)vector3 + 3);
}



void RenderContext::bindTexture(GLenum index, int texture) {
    oxglActiveTexture(index);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void RenderContext::bindTexture(const std::string& loc, GLuint active, const GLuint tex)const {
    GLuint location = glGetUniformLocation(_program, loc.c_str());
    glActiveTexture(GL_TEXTURE0 + active);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(location, active);
}

void RenderContext::bindTexture(GLenum index, int texture, std::string loc, int pos) const {
    int tex_a_location = oxglGetUniformLocation(_program, loc.c_str());
    oxglUniform1i(tex_a_location, pos);

    oxglActiveTexture(index);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void RenderContext::bindValue(const std::string &&loc, float f) const {
    GLint ul = oxglGetUniformLocation(_program, loc.c_str());
    oxglUniform1f(ul, f);
}

void RenderContext::bindValue(const std::string &&loc, const bool b) const {
    GLint ul = glGetUniformLocation(_program, loc.c_str());
    glUniform1i(ul, b);
}

void RenderContext::bindValue(const std::string &&loc, const Vector2 &v) const {
    GLint ul = glGetUniformLocation(_program, loc.c_str());
    glUniform2f(ul, v.x, v.y);
}

void RenderContext::bindValue(const std::string &&loc, const Vector3 &v) const {
    GLint ul = glGetUniformLocation(_program, loc.c_str());
    glUniform3f(ul, v.x, v.y, v.z);
}

void RenderContext::bindQuadVertices(int vbo, size_t size) const {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    GLuint location = glGetAttribLocation(_program, "aVertex");
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

//void RenderContext::bindValue(const string& loc, const glm::mat3x3& m) const {// TODO set matrix
//    GLint ul = glGetUniformLocation(activeProgram, loc.c_str());
//    glUniformMatrix3fv(ul, 1, GL_FALSE, glm::value_ptr(m));
//}

void RenderContext::Draw(const size_t count) const { //Note that this is the sizeof size. might have to change
    glDrawArrays(GL_TRIANGLES, 0, count);
}


