#pragma once

#include "test.h"

#include "light/RenderContext.h"
#include "light/LightSprite.h"
#include "light/LightBackground.h"

#include "light/Emitter.h"
#include "light/IDisplayLight.h"

DECLARE_SMART(OpenGLSprite, spOpenGLSprite);

DECLARE_SMART(TestBuffer, spTestBuff);

const char *vertexQUAD = R"(
in vec3 aVertex;
out vec2 vST;
void main() {
    vST = ((aVertex + 1) / 2).xy;
    gl_Position = vec4(aVertex.xy, 0, 1);
}
		)";

const char *fragmentFinalBlend = R"(
uniform sampler2D uColor, uIntensity;

in vec2 vST;

out vec4 FragColor;

void main() {
    FragColor.rgb = texture(uColor, vST, 0).rgb * (vec3(.01,.02,.05) + texture2D(uIntensity, vST, 0).rgb);
    FragColor.a = 1;
})";


const char *fragmentshadowCombine = R"(
void main() {
})";

const char *fragmentshadowGen = R"(
uniform bool uUnmask;

layout(location=0)out vec4 Mask;

//Mask out the triangles.
void main() {
    if (uUnmask) {
        Mask.rgb = vec3(0,1,1);
        Mask.a = 1;
    }
    else {
        Mask.rgb = vec3(1,0,0);
        Mask.a = 1;
    }
})";

const char *geomshadowGen = R"(
uniform vec3 uLightPosition;
uniform mat3 uViewportTransform;
uniform bool uUnmask;

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

//For now we'll just assume framebuffer's 800x600...

//ShadowGen: Projects the fragments onto the light radius' circle, and emits
//triangles to cover the occluded area.
//This is drawn on top of a circle-gradient texture framebuffer.
//(alpha: lightStrength->0, rgb: lightColor)
//This gets blended with the /.
void main() {
    vec3 lp = vec3(uLightPosition.x, uLightPosition.y, 0);

    //FIXME: Something's going wrong with our clip coord transform.
    //This should just be uViewportTransform * lp
    vec2 l = (uViewportTransform * lp).xy + vec2(-1, 1);

    vec2 v0 = gl_in[0].gl_Position.xy;
    vec2 v1 = gl_in[1].gl_Position.xy;
    vec2 v2 = gl_in[2].gl_Position.xy;
    vec2 v3 = v0 + (normalize(v0 - l) * vec2(800, 800));
    vec2 v4 = v1 + (normalize(v1 - l) * vec2(800, 800));
    vec2 v5 = v2 + (normalize(v2 - l) * vec2(800, 800));

    gl_Position = vec4(0,0,0,1);

    gl_Position.xy = v0; EmitVertex();
    gl_Position.xy = v1; EmitVertex();
    gl_Position.xy = v2; EmitVertex();
    EndPrimitive();
    if (uUnmask) return;

    gl_Position.xy = v0; EmitVertex();
    gl_Position.xy = v1; EmitVertex();
    gl_Position.xy = v3; EmitVertex();
    EndPrimitive();

    gl_Position.xy = v3; EmitVertex();
    gl_Position.xy = v1; EmitVertex();
    gl_Position.xy = v4; EmitVertex();
    EndPrimitive();
    gl_Position.xy = v1; EmitVertex();
    gl_Position.xy = v2; EmitVertex();
    gl_Position.xy = v4; EmitVertex();
    EndPrimitive();
    gl_Position.xy = v2; EmitVertex();
    gl_Position.xy = v4; EmitVertex();
    gl_Position.xy = v5; EmitVertex();
    EndPrimitive();
})";

const char *vertlightComp = R"(
in vec3 aVertex;

uniform sampler2D uNormal;

uniform vec2 uCameraPosition;
out vec2 vPositionUnits;
out vec2 vST;

void main() {
    vec2 size = textureSize(uNormal, 0);

    vec2 pxClip = aVertex.xy;
    vec2 vPositionUnits = ((pxClip + 1) / 2) * size;

    vPositionUnits -= uCameraPosition;

    vST = (pxClip + 1) / 2;

    gl_Position.xy = aVertex.xy;
    gl_Position.z = 0; //FIXME: SHould setting depth values be allowed?
    gl_Position.w = 1;
})";

const char *fraglightComp = R"(uniform float uConstant, uLinear, uQuadratic;
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
})";

const char *vertexShaderDataLightSprite = R"(
          in vec3 aVertex;
uniform mat3 uViewportTransform;
uniform vec2 uCameraPosition;
uniform vec3 uPosition;
uniform sampler2D tSprite;

out vec2 vST;
out vec2 vPositionUnits;

void main() {
    vec2 size = textureSize(tSprite, 0);
    vec3 pxPosition = vec3(size.x * aVertex.x + uPosition.x,
                           size.y * aVertex.y + uPosition.y, 1);
    pxPosition.xy -= uCameraPosition;

    gl_Position = vec4((uViewportTransform * pxPosition).xy, uPosition.z, 1);

    //FIXME: This doesn't account for bits  outside of the camera view.
    vPositionUnits = pxPosition.xy;

    vST = vec2(aVertex.x, aVertex.y);
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

    int _spriteprogram, m_blend_program, m_light_accum_program, m_light_pass_program, m_shadow_program;

    GLuint vertexArray;

    GLuint m_quad_vbo;
    size_t m_quad_vbo_size;

    GLuint m_light_pass_fbo;
    GLuint m_sprite_gbuffer;
    GLuint m_sprite_gbuffer_color;
    GLuint m_sprite_gbuffer_render;
    GLuint m_light_pass_fbo_tex, m_light_pass_mask_fbo,
            m_light_pass_mask_fbo_tex, m_light_accum_fbo, m_light_accum_fbo_tex;

    GLuint m_sprite_gbuffer_normal;

    vector<IDisplayLight *> objects;
    vector<Emitter *> lights;

    OpenGLSprite() {

    }

    void loadAll() {
        AnimationFrame frame = resources.getResAnim("bg")->getFrame(0);//
        int texture = (int) (size_t) frame.getDiffuse().base->getHandle();

        AnimationFrame frame = resources.getResAnim("bg")->getFrame(0);//
        int texture = (int) (size_t) frame.getDiffuse().base->getHandle();

        auto LightSpr = new LightBackground(texture, texture);
        LightSpr->x(10);
        LightSpr->y(10);

        auto LightSpr2 = new LightBackground(texture, texture);
        LightSpr2->x(512);
        LightSpr2->y(512);

        objects.push_back(LightSpr);
        objects.push_back(LightSpr2);

        Emitter *m_mouse_emitter = new Emitter(Vector3(200, 200, 10), 800, Vector3(1, 0.9, 0.9));

        lights.push_back(m_mouse_emitter);

        for (auto iter = objects.begin(); iter != objects.end(); iter++) {
            (*iter)->LoadResources();
        }

        glGenVertexArrays(1, &vertexArray);
        glBindVertexArray(vertexArray);

        //Make fullscreen quad vbo.
        Vector3 verts[6] = {
                Vector3(-1, -1, 1),
                Vector3(1, -1, 1),
                Vector3(-1, 1, 1),
                Vector3(1, -1, 1),
                Vector3(-1, 1, 1),
                Vector3(1, 1, 1)
        };

        m_quad_vbo_size = sizeof(verts);

        glGenBuffers(1, &m_quad_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_quad_vbo_size, &verts[0], GL_STATIC_DRAW);

        initPrograms();
        initBuffers();
//
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }

    void beginDrawSprites() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_sprite_gbuffer);
        oxglUseProgram(_spriteprogram);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    int m_width = 1;
    int m_height = 1;

    void initPrograms() {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        m_width = (float) viewport[2];
        m_height = (float) viewport[3];

        int vs = ShaderProgramGL::createShader(GL_VERTEX_SHADER, vertexShaderDataLightSprite);
        int fs = ShaderProgramGL::createShader(GL_FRAGMENT_SHADER, fragmentShaderDataLightSprite);

        _spriteprogram = oxglCreateProgram();
        oxglAttachShader(_spriteprogram, vs);
        oxglAttachShader(_spriteprogram, fs);;

//
        CHECKGL();

        oxglLinkProgram(_spriteprogram);
        oxglUseProgram(_spriteprogram);

//        MatrixT<float> display(2 / m_width, 0, 0, 0.0f, // first column
//                               0.0f, -2 / m_height, 0.0f, 0.0f, // second column
//                               -1.0f, 1.0f, 1.0f, 0.0f, // third column
//                               0.0f, 0.0f, 0.0f, 0.0f);

        GLfloat matrix[] = {
                2 / m_width, 0, 0,
                0, 2 / -m_height, 0,
                -1, 1, 1};

        //And uniforms.
        GLint mp = glGetUniformLocation(_spriteprogram, "uViewportTransform");
        oxglUniformMatrix4fv(mp, 1, GL_FALSE, matrix);
        GLint cp = glGetUniformLocation(_spriteprogram, "uCameraPosition");
        oxglUniform2f(cp, 0, 0);

        int blend_vsh = ShaderProgramGL::createShader(GL_VERTEX_SHADER, vertexQUAD);
        int blend_fsh = ShaderProgramGL::createShader(GL_FRAGMENT_SHADER, fragmentFinalBlend);

        m_blend_program = oxglCreateProgram();
        glAttachShader(m_blend_program, blend_vsh);
        glAttachShader(m_blend_program, blend_fsh);

        glLinkProgram(m_blend_program);
        oxglUseProgram(m_blend_program);

        int light_accum_vsh = ShaderProgramGL::createShader(GL_VERTEX_SHADER, vertexQUAD);
        int light_accum_fsh = ShaderProgramGL::createShader(GL_FRAGMENT_SHADER, fragmentshadowCombine);

        m_light_accum_program = oxglCreateProgram();
        glAttachShader(m_light_accum_program, light_accum_vsh);
        glAttachShader(m_light_accum_program, light_accum_fsh);

        oxglLinkProgram(m_light_accum_program);
        oxglUseProgram(m_light_accum_program);

        int lightvs = ShaderProgramGL::createShader(GL_VERTEX_SHADER, vertlightComp);
        int lightfr = ShaderProgramGL::createShader(GL_FRAGMENT_SHADER, fraglightComp);

        m_light_pass_program = oxglCreateProgram();
        glAttachShader(m_light_pass_program, lightvs);
        glAttachShader(m_light_pass_program, lightfr);

        oxglLinkProgram(m_light_pass_program);
        oxglUseProgram(m_light_pass_program);
        CHECKGL();
        int shadow_vsh = ShaderProgramGL::createShader(GL_VERTEX_SHADER, vertexShaderDataLightSprite);
        int shadow_fsh = ShaderProgramGL::createShader(GL_FRAGMENT_SHADER, fragmentshadowGen);
        int shadow_gsh = ShaderProgramGL::createShader(GL_GEOMETRY_SHADER, geomshadowGen);

        m_shadow_program = glCreateProgram();
        glAttachShader(m_shadow_program, shadow_vsh);
        glAttachShader(m_shadow_program, shadow_fsh);
        glAttachShader(m_shadow_program, shadow_gsh);

        glLinkProgram(m_shadow_program);
        oxglUseProgram(m_shadow_program);
        CHECKGL();

        GLint mp2 = glGetUniformLocation(m_shadow_program, "uViewportTransform");
        oxglUniformMatrix4fv(mp2, 1, GL_FALSE, matrix);

        oxglDetachShader(_spriteprogram, vs);
        oxglDetachShader(_spriteprogram, fs);


        oxglDetachShader(m_blend_program, blend_vsh);
        oxglDetachShader(m_blend_program, blend_fsh);

        oxglDetachShader(m_light_accum_program, light_accum_vsh);
        oxglDetachShader(m_light_accum_program, light_accum_fsh);

        oxglDetachShader(m_light_pass_program, lightvs);
        oxglDetachShader(m_light_pass_program, lightfr);


        oxglDetachShader(m_shadow_program, shadow_vsh);
        oxglDetachShader(m_shadow_program, shadow_fsh);
        oxglDetachShader(m_shadow_program, shadow_gsh);

        oxglDeleteShader(_spriteprogram);
        oxglDeleteShader(_spriteprogram);;

        oxglDeleteShader(blend_vsh);
        oxglDeleteShader(blend_fsh);;

        oxglDeleteShader(m_light_accum_program);
        oxglDeleteShader(m_light_accum_program);;

        oxglDeleteShader(lightvs);
        oxglDeleteShader(lightfr);;

        oxglDeleteShader(shadow_vsh);
        oxglDeleteShader(shadow_fsh);;
        oxglDeleteShader(shadow_gsh);;

    }

    void initBuffers() {
        glGenFramebuffers(1, &m_sprite_gbuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_sprite_gbuffer);

        //No need for precision.
        glGenTextures(1, &m_sprite_gbuffer_color);
        glBindTexture(GL_TEXTURE_2D, m_sprite_gbuffer_color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_sprite_gbuffer_color, 0);
        CHECKGL();
        glGenTextures(1, &m_sprite_gbuffer_normal);
        glBindTexture(GL_TEXTURE_2D, m_sprite_gbuffer_normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        CHECKGL();
        glGenRenderbuffers(1, &m_sprite_gbuffer_render);
        glBindRenderbuffer(GL_RENDERBUFFER, m_sprite_gbuffer_render);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_sprite_gbuffer_render);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_sprite_gbuffer_normal, 0);

        GLenum spriteDrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, spriteDrawBuffers);

        //generate light occlusion pass framebuffer.
        //the one that we draw one light to
        glGenFramebuffers(1, &m_light_pass_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_light_pass_fbo);

        glGenTextures(1, &m_light_pass_fbo_tex);
        glBindTexture(GL_TEXTURE_2D, m_light_pass_fbo_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_light_pass_fbo_tex, 0);
        CHECKGL();
        //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_light_pass_mask_fbo_tex, 0);
        GLenum lightDrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, lightDrawBuffers);

        glGenFramebuffers(1, &m_light_pass_mask_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_light_pass_mask_fbo);

        glGenTextures(1, &m_light_pass_mask_fbo_tex); //This could probably be lower precision.
        glBindTexture(GL_TEXTURE_2D, m_light_pass_mask_fbo_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        CHECKGL();
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_light_pass_mask_fbo_tex, 0);
        GLenum lightMaskDrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, lightMaskDrawBuffers);
        CHECKGL();
        //generate lights framebuffer.
        //the one we recombine our stuff into.
        glGenFramebuffers(1, &m_light_accum_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_light_accum_fbo);

        glGenTextures(1, &m_light_accum_fbo_tex);
        glBindTexture(GL_TEXTURE_2D, m_light_accum_fbo_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_light_accum_fbo_tex, 0);
        GLenum lightAccumDrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, lightAccumDrawBuffers);

        CHECKGL();
    }

    ~OpenGLSprite() {
        oxglDeleteProgram(_spriteprogram);
    }

    RenderContext c = RenderContext();
    RenderContext lightGenctx = RenderContext();
    RenderContext blendCtx = RenderContext();
    RenderContext lightCtx = RenderContext();
    RenderContext lightCombine = RenderContext();
    RenderContext shadowCtx = RenderContext();

    void doRender(const RenderState &rs) {
        Material::null->apply();

        glBindVertexArray(vertexArray);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //start scene
        glBindFramebuffer(GL_FRAMEBUFFER, m_light_accum_fbo);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, m_sprite_gbuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, m_light_pass_fbo);
        glClear(GL_COLOR_BUFFER_BIT);

        beginDrawSprites();
//
        c.useProgram(_spriteprogram);

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        AnimationFrame frame = resources.getResAnim("bg")->getFrame(0);//
//
        int texture = (int) (size_t) frame.getDiffuse().base->getHandle();
        for (auto iter = objects.begin(); iter != objects.end(); iter++) {
            (*iter)->Draw(c,texture,texture);
        }
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        //This is kinda hacky because I decided to store the framebuffers in the render object.
        //Ideally I'd do more of this in the scene.
        for (auto light = lights.begin(); light != lights.end(); light++) {
            //Set the framebuffer.
            glBindFramebuffer(GL_FRAMEBUFFER, m_light_pass_fbo);
            glClear(GL_COLOR_BUFFER_BIT);
            lightCtx.useProgram(m_light_pass_program);
            lightCtx.bindVertices(m_quad_vbo, m_quad_vbo_size);
            //Set the light uniforms
            lightCtx.bindValue("uLightColor", (*light)->color());
            lightCtx.bindValue("uLightPosition", (*light)->position());
            lightCtx.bindValue("uConstant", (*light)->constant());
            lightCtx.bindValue("uLinear", (*light)->linear());
            lightCtx.bindValue("uQuadratic", (*light)->quadratic());
            lightCtx.bindValue("uRadius", (*light)->radius());

            lightCtx.bindValue("uCameraPosition", Vector2(0, 0));

            lightCtx.bindTexture("uNormal", 0, m_sprite_gbuffer_normal);

            //Draw the textured quad pass.
            lightCtx.Draw(6);

            glBindFramebuffer(GL_FRAMEBUFFER, m_light_pass_mask_fbo);
            glClear(GL_COLOR_BUFFER_BIT);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            //Draw the shadow layer over it.
            shadowCtx.useProgram(m_shadow_program);
            shadowCtx.bindValue("uLightPosition", (*light)->position());
            shadowCtx.bindValue("uUnmask", false);
//            scene()->DrawShadowLayer(shadowCtx);
            shadowCtx.bindValue("uUnmask", true);
//            scene()->DrawShadowLayer(shadowCtx);

            //This is where blurring the mask layer would go.

            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            //Draw to the accumulated light framebuffer.
            glBindFramebuffer(GL_FRAMEBUFFER, m_light_accum_fbo);

            lightCombine.useProgram(m_light_accum_program);
            lightCombine.bindVertices(m_quad_vbo, m_quad_vbo_size);
            lightCombine.bindTexture("uIntensity", 0, m_light_pass_fbo_tex);
            lightCombine.bindTexture("uMask", 1, m_light_pass_mask_fbo_tex);
            lightCombine.Draw(6);
        }

        //Draw fullscreen quad.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        blendCtx.useProgram(m_blend_program);
        blendCtx.bindVertices(m_quad_vbo, m_quad_vbo_size);
        blendCtx.bindTexture("uColor", 0, m_sprite_gbuffer_color);
        blendCtx.bindTexture("uIntensity", 1, m_light_accum_fbo_tex);

        blendCtx.Draw(6);

        rsCache().reset();
//
    }
};


const char *testVertex = R"(
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_colour;

uniform mat4 matrix; // our matrix

out vec3 colour;

void main() {
	colour = vertex_colour;
	gl_Position = matrix * vec4(vertex_position, 1.0);
}
		)";

const char *testFragment = R"(
in vec3 colour;
out vec4 frag_colour;

void main() {
	frag_colour = vec4 (colour, 1.0);
}
		)";


/* OTHER STUFF GOES HERE NEXT */
GLfloat points[] = {0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f};

GLfloat colours[] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};


class TestOpenGL : public Test {
public:
    TestOpenGL() {
        spOpenGLSprite sprite = new OpenGLSprite;
        sprite->loadAll();

        content->addChild(sprite);
//        sprite->();

//        spSprite testBoth = new Sprite();
//        testBoth->setResAnim(resources.getResAnim("batterfly"));
//        content->addChild(testBoth);

    }
};


class TestBuffer : public Sprite {
public:
    int _spriteprogram;
    GLuint vao;

    TestBuffer() {
    }

    void init() {
        glEnable(GL_DEPTH_TEST); // enable depth-testing
        glDepthFunc(GL_LESS);         // depth-testing interprets a smaller value as "closer"

        int vs = ShaderProgramGL::createShader(GL_VERTEX_SHADER, testVertex);
        int fs = ShaderProgramGL::createShader(GL_FRAGMENT_SHADER, testFragment);

        _spriteprogram = oxglCreateProgram();
        oxglAttachShader(_spriteprogram, vs);
        oxglAttachShader(_spriteprogram, fs);;

//
        CHECKGL();

        oxglLinkProgram(_spriteprogram);


        oxglDetachShader(_spriteprogram, vs);
        oxglDetachShader(_spriteprogram, fs);

        oxglDeleteShader(vs);
        oxglDeleteShader(fs);;

        glEnable(GL_CULL_FACE); // cull face
        glCullFace(GL_BACK);        // cull back face
        glFrontFace(GL_CW);            // GL_CCW for counter clock-wise

    }

    void doRender(const RenderState &rs) {
// wipe the drawing surface clear

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, 800, 800);

        GLuint points_vbo;
        glGenBuffers(1, &points_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), points, GL_STATIC_DRAW);

        GLuint colours_vbo;
        glGenBuffers(1, &colours_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colours, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        //
        MatrixT<float> t(1.0f, 0.0f, 0.0f, 0.0f, // first column
                         0.0f, 1.0f, 0.0f, 0.0f, // second column
                         0.0f, 0.0f, 1.0f, 0.0f, // third column
                         0.5f, 0.0f, 0.0f, 1.0f);

        int matrix_location = glGetUniformLocation(_spriteprogram, "matrix");
        glUseProgram(_spriteprogram);
        glUniformMatrix4fv(matrix_location, 1, GL_FALSE, t.ml);

        glEnable(GL_CULL_FACE); // cull face
        glCullFace(GL_BACK);        // cull back face
        glFrontFace(GL_CW);

        // Note: this call is not necessary, but I like to do it anyway before any
        // time that I call glDrawArrays() so I never use the wrong shader programme
        //
        // Note: this call is related to the most recently 'used' shader programme
        oxglUniformMatrix4fv(matrix_location, 1, GL_FALSE, t.ml);

        //
        // Note: this call is not necessary, but I like to do it anyway before any
        // time that I call glDrawArrays() so I never use the wrong vertex data
        glBindVertexArray(vao);
        // draw points 0-3 from the currently bound VAO with current in-use shader
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
};