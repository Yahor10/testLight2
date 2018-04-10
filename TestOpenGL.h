#pragma once

#include "test.h"

#include "light/RenderContext.h"
#include "light/LightSprite.h"
#include "light/Emitter.h"

DECLARE_SMART(OpenGLSprite, spOpenGLSprite);

const char *vertexShaderDataLightGen = R"(
         in vec3 aVertex;

uniform sampler2D uNormal;
uniform vec2 uCameraPosition;

out vec2 vPositionUnits;
out vec2 vST;

void main() {
    vec2 size = textureSize(uNormal, 0);

    vec2 pxClip = aVertex.xy;
    vec2 vPositionUnits = ((pxClip + 1) / 2) * size;

   // vPositionUnits -= uCameraPosition;

    vST = (pxClip + 1) / 2;

    gl_Position.xy = aVertex.xy;
    gl_Position.z = 0; //FIXME: SHould setting depth values be allowed?
    gl_Position.w = 1;
}
		)";

const char *fragShaderDataLightGen = R"(
uniform float uConstant, uLinear, uQuadratic;
uniform float uRadius;

uniform vec3 uLightPosition, uLightColor;

uniform sampler2D uNormal;

out vec2 vST;
out vec2 vPositionUnits;

layout(location=0) out vec4 Color;

void main() {
    vec3 vp = vec3(gl_FragCoord.x, 600 - gl_FragCoord.y , 0);

    //Find the point we're at.
    float distance = length(vec3(vp) - uLightPosition);
    float dn = (distance / uRadius );

    float intensity = 1 / (uConstant + (dn * uLinear) + (dn * dn * uQuadratic));

    vec3 nm = texture2D(uNormal, vST, 0).rgb;

    vec3 n = normalize((2 * nm) - 1);
    vec3 l = normalize(uLightPosition - vp);

    float diffuseStrength = max(0, dot(l, n));

    Color.rgb = uLightColor * diffuseStrength;
    Color.a = clamp(intensity, 0, 1);
}
		)";


const char *vertexShaderDataLightSprite = R"(
          in vec3 a_position;
          in vec2 a_uv;

uniform mat4 projection;
uniform vec2 uCameraPosition;
uniform vec3 uPosition;
uniform sampler2D tSprite;

out vec2 vST;
out vec2 vPositionUnits;

void main() {
       vec4 position = vec4(a_position, 1.0);
       gl_Position = projection * position;
      vST = a_uv;
}
		)";

const char *fragmentShaderDataLightSprite = R"(
         uniform sampler2D tSprite,tNormal;
//        uniform vec2 uCameraPosition;

    in vec2 vST;
    in vec2 vPositionUnits;

layout(location=0) out vec4 Color;
layout(location=1) out vec4 Normal;

void main() {
    Normal.rgb = texture2D(tNormal, vST).rgb;
    Normal.a = 1;

    Color.rgb = texture2D(tSprite, vST).rgb;
    Color.a = 1;
}
		)";


class OpenGLSprite : public Sprite {
public:

    int _spriteprogram, _lightprogram;

    GLuint vertexArray;

    GLuint m_quad_vbo;
    size_t m_quad_vbo_size;

    GLuint m_light_pass_fbo;
    GLuint m_sprite_gbuffer;

    GLuint m_sprite_gbuffer_normal;


    OpenGLSprite() {
        load();

        initPrograms();
        initBuffers();

        glEnable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);


        CHECKGL();
    }

    void load() {
//        glGenVertexArrays(1, &vertexArray);
//        glBindVertexArray(vertexArray);
    }

    void beginDrawSprites() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_sprite_gbuffer);
        glUseProgram(_spriteprogram);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void initPrograms() {

        int vs = ShaderProgramGL::createShader(GL_VERTEX_SHADER, vertexShaderDataLightSprite);
        int fs = ShaderProgramGL::createShader(GL_FRAGMENT_SHADER, fragmentShaderDataLightSprite);

        int vslight = ShaderProgramGL::createShader(GL_VERTEX_SHADER, vertexShaderDataLightGen);
        int fslight = ShaderProgramGL::createShader(GL_FRAGMENT_SHADER, fragShaderDataLightGen);


        _spriteprogram = oxglCreateProgram();
        oxglAttachShader(_spriteprogram, vs);
        oxglAttachShader(_spriteprogram, fs);

        _lightprogram = oxglCreateProgram();
        oxglAttachShader(_lightprogram, vslight);
        oxglAttachShader(_lightprogram, fslight);

        // TODO init shaders and global uniforms
        CHECKGL();
//
        oxglBindAttribLocation(_spriteprogram, 0, "a_position");
        oxglBindAttribLocation(_spriteprogram, 1, "a_uv");
//
        CHECKGL();

        oxglLinkProgram(_spriteprogram);
        oxglLinkProgram(_lightprogram);


        oxglDetachShader(_spriteprogram, vs);
        oxglDetachShader(_spriteprogram, fs);

        oxglDeleteShader(vs);
        oxglDeleteShader(fs);
//
        oxglDeleteShader(vslight);
        oxglDeleteShader(fslight);
    }

    void initBuffers() {
        int m_width = 800;
        int m_height = 600;

        glGenTextures(1, &m_sprite_gbuffer_normal);
        glBindTexture(GL_TEXTURE_2D, m_sprite_gbuffer_normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        //generate light occlusion pass framebuffer.
        //the one that we draw one light to
        glGenFramebuffers(1, &m_light_pass_fbo);
//        glBindFramebuffer(GL_FRAMEBUFFER, m_light_pass_fbo);
    }

    ~OpenGLSprite() {
        oxglDeleteProgram(_spriteprogram);
        oxglDeleteProgram(_lightprogram);
    }

    RenderContext c = RenderContext();
    RenderContext lightGenctx = RenderContext();

    void doRender(const RenderState &rs) {
        Material::null->apply();

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        IVideoDriver *driver = IVideoDriver::instance;
        c.useProgram(_spriteprogram);

        Matrix m = Matrix(rs.transform) * STDRenderer::instance->getViewProjection();

        int projLocation = oxglGetUniformLocation(_spriteprogram, "projection");
        oxglUniformMatrix4fv(projLocation, 1, GL_FALSE, m.ml);

        AnimationFrame frame = resources.getResAnim("bg")->getFrame(0);
        const RectF &srcRect = frame.getSrcRect();


        int texture = (int) (size_t) frame.getDiffuse().base->getHandle();
        auto LightSpr = new LightSprite(texture, texture);

        myVertex vertices[4];
        myVertex *v = vertices;

        v->pos = Vector3(-100, -100, 0);
        v->uv = srcRect.getLeftTop();
        ++v;

        v->pos = Vector3(100, -100, 0);
        v->uv = srcRect.getRightTop();
        ++v;

        v->pos = Vector3(100, 100, 0);
        v->uv = srcRect.getRightBottom();
        ++v;

        v->pos = Vector3(-100, 100, 0);
        v->uv = srcRect.getLeftBottom();
        ++v;

        c.bindPosVertices(0, sizeof(myVertex), vertices);
        c.bindUVVertices(1, sizeof(myVertex), vertices);

        CHECKGL();

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND); // render sprites

        LightSpr->Draw(c);

        glDisable(GL_DEPTH_TEST);
        CHECKGL();
//
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        //        // End func
        oxglDisableVertexAttribArray(0);// TODO refactor
        oxglDisableVertexAttribArray(1);
//
//        glBindFramebuffer(GL_FRAMEBUFFER, m_light_pass_fbo); // render light not working for alone ???
//        glClear(GL_COLOR_BUFFER_BIT);

//        auto light = new Emitter(Vector3(200, 200, 10), 800, Vector3(1, 0.9, 0.9));
//        lightGenctx.useProgram(_lightprogram);

//        glBindFramebuffer(GL_FRAMEBUFFER, m_light_pass_fbo);
//        glClear(GL_COLOR_BUFFER_BIT);

//        lightGenctx.bindVertices(m_quad_vbo, m_quad_vbo_size);
//        //Set the light uniforms
//        lightGenctx.bindValue("uLightColor", light->color());
//        lightGenctx.bindValue("uLightPosition", light->position());
//        lightGenctx.bindValue("uConstant", light->constant());
//        lightGenctx.bindValue("uLinear", light->linear());
//        lightGenctx.bindValue("uQuadratic", light->quadratic());
//        lightGenctx.bindValue("uRadius", light->radius());
//
//        lightGenctx.bindValue("uCameraPosition", Vector2(0, 0));
//
//        lightGenctx.bindTexture("uNormal", 0, m_sprite_gbuffer_normal);
//
//        //Draw the textured quad pass.
//        lightGenctx.Draw(6);
//


    }
};


class TestOpenGL : public Test {
public:
    TestOpenGL() {
        spOpenGLSprite sprite = new OpenGLSprite;
        sprite->setPosition(getStage()->getSize() / 2);
        sprite->setScale(3);
        content->addChild(sprite);

//        spSprite testBoth = new Sprite();
//        testBoth->setResAnim(resources.getResAnim("batterfly"));
//        content->addChild(testBoth);

    }
};