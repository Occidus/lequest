/************************************************************************************

Filename    :   VrCubeWorld_NativeActivity.c
Content     :   This sample uses the Android NativeActivity class. This sample does
                not use the application framework.
                This sample only uses the VrApi.
Created     :   March, 2015
Authors     :   J.M.P. van Waveren

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

*************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h> // for prctl( PR_SET_NAME )
#include <android/log.h>
#include <android/window.h> // for AWINDOW_FLAG_KEEP_SCREEN_ON
#include <android/native_window_jni.h> // for native window JNI
#include <android_native_app_glue.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include "render.h"
#include "linear.h"
#include "learningvr.h"

using namespace r3;

#if !defined(EGL_OPENGL_ES3_BIT_KHR)
#define EGL_OPENGL_ES3_BIT_KHR 0x0040
#endif



// EXT_texture_border_clamp
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER 0x812D
#endif

#ifndef GL_TEXTURE_BORDER_COLOR
#define GL_TEXTURE_BORDER_COLOR 0x1004
#endif

#include "VrApi.h"
#include "VrApi_Helpers.h"
#include "VrApi_SystemUtils.h"
#include "VrApi_Input.h"

#define DEBUG 1
#define OVR_LOG_TAG "VrCubeWorld"

#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, OVR_LOG_TAG, __VA_ARGS__)
#if DEBUG
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, OVR_LOG_TAG, __VA_ARGS__)
#else
#define ALOGV(...)
#endif

static const int CPU_LEVEL = 2;
static const int GPU_LEVEL = 3;

/*
================================================================================

System Clock Time

================================================================================
*/

static double GetTimeInSeconds() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec * 1e9 + now.tv_nsec) * 0.000000001;
}

const Matrix4f& ToR3(const ovrMatrix4f & om) {
    return *reinterpret_cast<const Matrix4f*>(&om);
}

const Posef& ToR3(const ovrPosef & op) {
    return *reinterpret_cast<const Posef*>(&op);
}

const Vec3f& ToR3(const ovrVector3f & op) {
    return *reinterpret_cast<const Vec3f*>(&op);
}

/*
================================================================================

OpenGL-ES Utility Functions

================================================================================
*/

struct OpenGLExtensions_t {
    bool EXT_texture_border_clamp; // GL_EXT_texture_border_clamp, GL_OES_texture_border_clamp
};

OpenGLExtensions_t glExtensions;

static void EglInitExtensions() {
    const char* allExtensions = (const char*)glGetString(GL_EXTENSIONS);
    if (allExtensions != nullptr) {
        glExtensions.EXT_texture_border_clamp =
            strstr(allExtensions, "GL_EXT_texture_border_clamp") ||
            strstr(allExtensions, "GL_OES_texture_border_clamp");
    }
}

static const char* EglErrorString(const EGLint error) {
    switch (error) {
        case EGL_SUCCESS:
            return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED:
            return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS:
            return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC:
            return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE:
            return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONTEXT:
            return "EGL_BAD_CONTEXT";
        case EGL_BAD_CONFIG:
            return "EGL_BAD_CONFIG";
        case EGL_BAD_CURRENT_SURFACE:
            return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY:
            return "EGL_BAD_DISPLAY";
        case EGL_BAD_SURFACE:
            return "EGL_BAD_SURFACE";
        case EGL_BAD_MATCH:
            return "EGL_BAD_MATCH";
        case EGL_BAD_PARAMETER:
            return "EGL_BAD_PARAMETER";
        case EGL_BAD_NATIVE_PIXMAP:
            return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW:
            return "EGL_BAD_NATIVE_WINDOW";
        case EGL_CONTEXT_LOST:
            return "EGL_CONTEXT_LOST";
        default:
            return "unknown";
    }
}

static const char* GlFrameBufferStatusString(GLenum status) {
    switch (status) {
        case GL_FRAMEBUFFER_UNDEFINED:
            return "GL_FRAMEBUFFER_UNDEFINED";
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "GL_FRAMEBUFFER_UNSUPPORTED";
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        default:
            return "unknown";
    }
}

#ifdef CHECK_GL_ERRORS

static const char* GlErrorString(GLenum error) {
    switch (error) {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        default:
            return "unknown";
    }
}

static void GLCheckErrors(int line) {
    for (int i = 0; i < 10; i++) {
        const GLenum error = glGetError();
        if (error == GL_NO_ERROR) {
            break;
        }
        ALOGE("GL error on line %d: %s", line, GlErrorString(error));
    }
}

#define GL(func) \
    func;        \
    GLCheckErrors(__LINE__);

#else // CHECK_GL_ERRORS

#define GL(func) func;

#endif // CHECK_GL_ERRORS

/*
================================================================================

ovrEgl

================================================================================
*/

struct ovrEgl {
    EGLint MajorVersion;
    EGLint MinorVersion;
    EGLDisplay Display;
    EGLConfig Config;
    EGLSurface TinySurface;
    EGLSurface MainSurface;
    EGLContext Context;
};

static void ovrEgl_Clear(ovrEgl* egl) {
    egl->MajorVersion = 0;
    egl->MinorVersion = 0;
    egl->Display = 0;
    egl->Config = 0;
    egl->TinySurface = EGL_NO_SURFACE;
    egl->MainSurface = EGL_NO_SURFACE;
    egl->Context = EGL_NO_CONTEXT;
}

static void ovrEgl_CreateContext(ovrEgl* egl, const ovrEgl* shareEgl) {
    if (egl->Display != 0) {
        return;
    }

    egl->Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    ALOGV("        eglInitialize( Display, &MajorVersion, &MinorVersion )");
    eglInitialize(egl->Display, &egl->MajorVersion, &egl->MinorVersion);
    // Do NOT use eglChooseConfig, because the Android EGL code pushes in multisample
    // flags in eglChooseConfig if the user has selected the "force 4x MSAA" option in
    // settings, and that is completely wasted for our warp target.
    const int MAX_CONFIGS = 1024;
    EGLConfig configs[MAX_CONFIGS];
    EGLint numConfigs = 0;
    if (eglGetConfigs(egl->Display, configs, MAX_CONFIGS, &numConfigs) == EGL_FALSE) {
        ALOGE("        eglGetConfigs() failed: %s", EglErrorString(eglGetError()));
        return;
    }
    const EGLint configAttribs[] = {EGL_RED_SIZE,
                                    8,
                                    EGL_GREEN_SIZE,
                                    8,
                                    EGL_BLUE_SIZE,
                                    8,
                                    EGL_ALPHA_SIZE,
                                    8, // need alpha for the multi-pass timewarp compositor
                                    EGL_DEPTH_SIZE,
                                    0,
                                    EGL_STENCIL_SIZE,
                                    0,
                                    EGL_SAMPLES,
                                    0,
                                    EGL_NONE};
    egl->Config = 0;
    for (int i = 0; i < numConfigs; i++) {
        EGLint value = 0;

        eglGetConfigAttrib(egl->Display, configs[i], EGL_RENDERABLE_TYPE, &value);
        if ((value & EGL_OPENGL_ES3_BIT_KHR) != EGL_OPENGL_ES3_BIT_KHR) {
            continue;
        }

        // The pbuffer config also needs to be compatible with normal window rendering
        // so it can share textures with the window context.
        eglGetConfigAttrib(egl->Display, configs[i], EGL_SURFACE_TYPE, &value);
        if ((value & (EGL_WINDOW_BIT | EGL_PBUFFER_BIT)) != (EGL_WINDOW_BIT | EGL_PBUFFER_BIT)) {
            continue;
        }

        int j = 0;
        for (; configAttribs[j] != EGL_NONE; j += 2) {
            eglGetConfigAttrib(egl->Display, configs[i], configAttribs[j], &value);
            if (value != configAttribs[j + 1]) {
                break;
            }
        }
        if (configAttribs[j] == EGL_NONE) {
            egl->Config = configs[i];
            break;
        }
    }
    if (egl->Config == 0) {
        ALOGE("        eglChooseConfig() failed: %s", EglErrorString(eglGetError()));
        return;
    }
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    ALOGV("        Context = eglCreateContext( Display, Config, EGL_NO_CONTEXT, contextAttribs )");
    egl->Context = eglCreateContext(
        egl->Display,
        egl->Config,
        (shareEgl != nullptr) ? shareEgl->Context : EGL_NO_CONTEXT,
        contextAttribs);
    if (egl->Context == EGL_NO_CONTEXT) {
        ALOGE("        eglCreateContext() failed: %s", EglErrorString(eglGetError()));
        return;
    }
    const EGLint surfaceAttribs[] = {EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE};
    ALOGV("        TinySurface = eglCreatePbufferSurface( Display, Config, surfaceAttribs )");
    egl->TinySurface = eglCreatePbufferSurface(egl->Display, egl->Config, surfaceAttribs);
    if (egl->TinySurface == EGL_NO_SURFACE) {
        ALOGE("        eglCreatePbufferSurface() failed: %s", EglErrorString(eglGetError()));
        eglDestroyContext(egl->Display, egl->Context);
        egl->Context = EGL_NO_CONTEXT;
        return;
    }
    ALOGV("        eglMakeCurrent( Display, TinySurface, TinySurface, Context )");
    if (eglMakeCurrent(egl->Display, egl->TinySurface, egl->TinySurface, egl->Context) ==
        EGL_FALSE) {
        ALOGE("        eglMakeCurrent() failed: %s", EglErrorString(eglGetError()));
        eglDestroySurface(egl->Display, egl->TinySurface);
        eglDestroyContext(egl->Display, egl->Context);
        egl->Context = EGL_NO_CONTEXT;
        return;
    }
}

static void ovrEgl_DestroyContext(ovrEgl* egl) {
    if (egl->Display != 0) {
        ALOGE("        eglMakeCurrent( Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT )");
        if (eglMakeCurrent(egl->Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) ==
            EGL_FALSE) {
            ALOGE("        eglMakeCurrent() failed: %s", EglErrorString(eglGetError()));
        }
    }
    if (egl->Context != EGL_NO_CONTEXT) {
        ALOGE("        eglDestroyContext( Display, Context )");
        if (eglDestroyContext(egl->Display, egl->Context) == EGL_FALSE) {
            ALOGE("        eglDestroyContext() failed: %s", EglErrorString(eglGetError()));
        }
        egl->Context = EGL_NO_CONTEXT;
    }
    if (egl->TinySurface != EGL_NO_SURFACE) {
        ALOGE("        eglDestroySurface( Display, TinySurface )");
        if (eglDestroySurface(egl->Display, egl->TinySurface) == EGL_FALSE) {
            ALOGE("        eglDestroySurface() failed: %s", EglErrorString(eglGetError()));
        }
        egl->TinySurface = EGL_NO_SURFACE;
    }
    if (egl->Display != 0) {
        ALOGE("        eglTerminate( Display )");
        if (eglTerminate(egl->Display) == EGL_FALSE) {
            ALOGE("        eglTerminate() failed: %s", EglErrorString(eglGetError()));
        }
        egl->Display = 0;
    }
}

/*
================================================================================

ovrGeometry

================================================================================
*/

struct ovrVertexAttribPointer {
    GLint Index;
    GLint Size;
    GLenum Type;
    GLboolean Normalized;
    GLsizei Stride;
    const GLvoid* Pointer;
};

#define MAX_VERTEX_ATTRIB_POINTERS 3

struct ovrGeometry {
    GLuint VertexBuffer;
    GLuint IndexBuffer;
    GLuint VertexArrayObject;
    int VertexCount;
    int IndexCount;
    ovrVertexAttribPointer VertexAttribs[MAX_VERTEX_ATTRIB_POINTERS];
};

enum VertexAttributeLocation {
    VERTEX_ATTRIBUTE_LOCATION_POSITION,
    VERTEX_ATTRIBUTE_LOCATION_COLOR,
    VERTEX_ATTRIBUTE_LOCATION_UV,
    VERTEX_ATTRIBUTE_LOCATION_TRANSFORM
};

struct ovrVertexAttribute {
    enum VertexAttributeLocation location;
    const char* name;
};

static ovrVertexAttribute ProgramVertexAttributes[] = {
    {VERTEX_ATTRIBUTE_LOCATION_POSITION, "vertexPosition"},
    {VERTEX_ATTRIBUTE_LOCATION_COLOR, "vertexColor"},
    {VERTEX_ATTRIBUTE_LOCATION_UV, "vertexUv"},
    {VERTEX_ATTRIBUTE_LOCATION_TRANSFORM, "vertexTransform"}};

static void ovrGeometry_Clear(ovrGeometry* geometry) {
    geometry->VertexBuffer = 0;
    geometry->IndexBuffer = 0;
    geometry->VertexArrayObject = 0;
    geometry->VertexCount = 0;
    geometry->IndexCount = 0;
    for (int i = 0; i < MAX_VERTEX_ATTRIB_POINTERS; i++) {
        memset(&geometry->VertexAttribs[i], 0, sizeof(geometry->VertexAttribs[i]));
        geometry->VertexAttribs[i].Index = -1;
    }
}

static void ovrGeometry_CreateCube(ovrGeometry* geometry) {
    struct ovrCubeVertices {
        signed char positions[8][4];
        unsigned char colors[8][4];
    };

    static const ovrCubeVertices cubeVertices = {
        // positions
        {
            {-127, +127, -127, +127},
            {+127, +127, -127, +127},
            {+127, +127, +127, +127},
            {-127, +127, +127, +127}, // top
            {-127, -127, -127, +127},
            {-127, -127, +127, +127},
            {+127, -127, +127, +127},
            {+127, -127, -127, +127} // bottom
        },
        // colors
        {{255, 0, 0, 255},
         {255, 0, 0, 255},
         {0, 255, 0, 255},
         {0, 255, 0, 255},
         {0, 0, 255, 255},
         {0, 0, 255, 255},
         {255, 0, 0, 255},
         {255, 0, 0, 255}},
    };

    static const unsigned short cubeIndices[36] = {
        0, 2, 1, 2, 0, 3, // top
        4, 6, 5, 6, 4, 7, // bottom
        2, 6, 7, 7, 1, 2, // right
        0, 4, 5, 5, 3, 0, // left
        3, 5, 6, 6, 2, 3, // front
        0, 1, 7, 7, 4, 0 // back
    };

    geometry->VertexCount = 8;
    geometry->IndexCount = 36;

    geometry->VertexAttribs[0].Index = VERTEX_ATTRIBUTE_LOCATION_POSITION;
    geometry->VertexAttribs[0].Size = 4;
    geometry->VertexAttribs[0].Type = GL_BYTE;
    geometry->VertexAttribs[0].Normalized = true;
    geometry->VertexAttribs[0].Stride = sizeof(cubeVertices.positions[0]);
    geometry->VertexAttribs[0].Pointer = (const GLvoid*)offsetof(ovrCubeVertices, positions);

    geometry->VertexAttribs[1].Index = VERTEX_ATTRIBUTE_LOCATION_COLOR;
    geometry->VertexAttribs[1].Size = 4;
    geometry->VertexAttribs[1].Type = GL_UNSIGNED_BYTE;
    geometry->VertexAttribs[1].Normalized = true;
    geometry->VertexAttribs[1].Stride = sizeof(cubeVertices.colors[0]);
    geometry->VertexAttribs[1].Pointer = (const GLvoid*)offsetof(ovrCubeVertices, colors);

    GL(glGenBuffers(1, &geometry->VertexBuffer));
    GL(glBindBuffer(GL_ARRAY_BUFFER, geometry->VertexBuffer));
    GL(glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW));
    GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    GL(glGenBuffers(1, &geometry->IndexBuffer));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->IndexBuffer));
    GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

static void ovrGeometry_Destroy(ovrGeometry* geometry) {
    GL(glDeleteBuffers(1, &geometry->IndexBuffer));
    GL(glDeleteBuffers(1, &geometry->VertexBuffer));

    ovrGeometry_Clear(geometry);
}

static void ovrGeometry_CreateVAO(ovrGeometry* geometry) {
    GL(glGenVertexArrays(1, &geometry->VertexArrayObject));
    GL(glBindVertexArray(geometry->VertexArrayObject));

    GL(glBindBuffer(GL_ARRAY_BUFFER, geometry->VertexBuffer));

    for (int i = 0; i < MAX_VERTEX_ATTRIB_POINTERS; i++) {
        if (geometry->VertexAttribs[i].Index != -1) {
            GL(glEnableVertexAttribArray(geometry->VertexAttribs[i].Index));
            GL(glVertexAttribPointer(
                geometry->VertexAttribs[i].Index,
                geometry->VertexAttribs[i].Size,
                geometry->VertexAttribs[i].Type,
                geometry->VertexAttribs[i].Normalized,
                geometry->VertexAttribs[i].Stride,
                geometry->VertexAttribs[i].Pointer));
        }
    }

    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->IndexBuffer));

    GL(glBindVertexArray(0));
}

static void ovrGeometry_DestroyVAO(ovrGeometry* geometry) {
    GL(glDeleteVertexArrays(1, &geometry->VertexArrayObject));
}

/*
================================================================================

ovrProgram

================================================================================
*/

#define MAX_PROGRAM_UNIFORMS 8
#define MAX_PROGRAM_TEXTURES 8

struct ovrProgram {
    GLuint Program;
    GLuint VertexShader;
    GLuint FragmentShader;
    // These will be -1 if not used by the program.
    GLint UniformLocation[MAX_PROGRAM_UNIFORMS]; // ProgramUniforms[].name
    GLint UniformBinding[MAX_PROGRAM_UNIFORMS]; // ProgramUniforms[].name
    GLint Textures[MAX_PROGRAM_TEXTURES]; // Texture%i
};

struct ovrUniform {
    enum Index {
        MODEL,
        VIEW_ID,
        SCENE_MATRICES,
    } index;
    enum Type {
        VECTOR4,
        MATRIX4X4,
        INT,
        BUFFER,
    } type;
    const char* name;
};

static ovrUniform ProgramUniforms[] = {
    {ovrUniform::Index::MODEL, ovrUniform::Type::MATRIX4X4, "ModelMatrix"},
    {ovrUniform::Index::VIEW_ID, ovrUniform::Type::INT, "ViewID"},
    {ovrUniform::Index::SCENE_MATRICES, ovrUniform::Type::BUFFER, "SceneMatrices"},
};

static void ovrProgram_Clear(ovrProgram* program) {
    program->Program = 0;
    program->VertexShader = 0;
    program->FragmentShader = 0;
    memset(program->UniformLocation, 0, sizeof(program->UniformLocation));
    memset(program->UniformBinding, 0, sizeof(program->UniformBinding));
    memset(program->Textures, 0, sizeof(program->Textures));
}

static const char* programVersion = "#version 300 es\n";

static bool ovrProgram_Create(
    ovrProgram* program,
    const char* vertexSource,
    const char* fragmentSource) {
    GLint r;

    GL(program->VertexShader = glCreateShader(GL_VERTEX_SHADER));

    const char* vertexSources[2] = {
        programVersion,
        vertexSource};
    GL(glShaderSource(program->VertexShader, 2, vertexSources, 0));
    GL(glCompileShader(program->VertexShader));
    GL(glGetShaderiv(program->VertexShader, GL_COMPILE_STATUS, &r));
    if (r == GL_FALSE) {
        GLchar msg[4096];
        GL(glGetShaderInfoLog(program->VertexShader, sizeof(msg), 0, msg));
        ALOGE("%s\n%s\n", vertexSource, msg);
        return false;
    }

    const char* fragmentSources[2] = {programVersion, fragmentSource};
    GL(program->FragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
    GL(glShaderSource(program->FragmentShader, 2, fragmentSources, 0));
    GL(glCompileShader(program->FragmentShader));
    GL(glGetShaderiv(program->FragmentShader, GL_COMPILE_STATUS, &r));
    if (r == GL_FALSE) {
        GLchar msg[4096];
        GL(glGetShaderInfoLog(program->FragmentShader, sizeof(msg), 0, msg));
        ALOGE("%s\n%s\n", fragmentSource, msg);
        return false;
    }

    GL(program->Program = glCreateProgram());
    GL(glAttachShader(program->Program, program->VertexShader));
    GL(glAttachShader(program->Program, program->FragmentShader));

    // Bind the vertex attribute locations.
    for (int i = 0; i < sizeof(ProgramVertexAttributes) / sizeof(ProgramVertexAttributes[0]); i++) {
        GL(glBindAttribLocation(
            program->Program,
            ProgramVertexAttributes[i].location,
            ProgramVertexAttributes[i].name));
    }

    GL(glLinkProgram(program->Program));
    GL(glGetProgramiv(program->Program, GL_LINK_STATUS, &r));
    if (r == GL_FALSE) {
        GLchar msg[4096];
        GL(glGetProgramInfoLog(program->Program, sizeof(msg), 0, msg));
        ALOGE("Linking program failed: %s\n", msg);
        return false;
    }

    int numBufferBindings = 0;

    // Get the uniform locations.
    memset(program->UniformLocation, -1, sizeof(program->UniformLocation));
    for (int i = 0; i < sizeof(ProgramUniforms) / sizeof(ProgramUniforms[0]); i++) {
        const int uniformIndex = ProgramUniforms[i].index;
        if (ProgramUniforms[i].type == ovrUniform::Type::BUFFER) {
            GL(program->UniformLocation[uniformIndex] =
                   glGetUniformBlockIndex(program->Program, ProgramUniforms[i].name));
            program->UniformBinding[uniformIndex] = numBufferBindings++;
            GL(glUniformBlockBinding(
                program->Program,
                program->UniformLocation[uniformIndex],
                program->UniformBinding[uniformIndex]));
        } else {
            GL(program->UniformLocation[uniformIndex] =
                   glGetUniformLocation(program->Program, ProgramUniforms[i].name));
            program->UniformBinding[uniformIndex] = program->UniformLocation[uniformIndex];
        }
    }

    GL(glUseProgram(program->Program));

    // Get the texture locations.
    for (int i = 0; i < MAX_PROGRAM_TEXTURES; i++) {
        char name[32];
        sprintf(name, "Texture%i", i);
        program->Textures[i] = glGetUniformLocation(program->Program, name);
        if (program->Textures[i] != -1) {
            GL(glUniform1i(program->Textures[i], i));
        }
    }

    GL(glUseProgram(0));

    return true;
}

static void ovrProgram_Destroy(ovrProgram* program) {
    if (program->Program != 0) {
        GL(glDeleteProgram(program->Program));
        program->Program = 0;
    }
    if (program->VertexShader != 0) {
        GL(glDeleteShader(program->VertexShader));
        program->VertexShader = 0;
    }
    if (program->FragmentShader != 0) {
        GL(glDeleteShader(program->FragmentShader));
        program->FragmentShader = 0;
    }
}

static const char VERTEX_SHADER[] =
    "#define NUM_VIEWS 2\n"
    "uniform lowp int ViewID;\n"
    "#define VIEW_ID ViewID\n"
    "in vec3 vertexPosition;\n"
    "in vec4 vertexColor;\n"
    "in mat4 vertexTransform;\n"
    "uniform SceneMatrices\n"
    "{\n"
    "	uniform mat4 ViewMatrix[NUM_VIEWS];\n"
    "	uniform mat4 ProjectionMatrix[NUM_VIEWS];\n"
    "} sm;\n"
    "out vec4 fragmentColor;\n"
    "void main()\n"
    "{\n"
    "	gl_Position = sm.ProjectionMatrix[VIEW_ID] * ( sm.ViewMatrix[VIEW_ID] * ( vertexTransform * vec4( vertexPosition * 0.1, 1.0 ) ) );\n"
    "	fragmentColor = vertexColor;\n"
    "}\n";

static const char FRAGMENT_SHADER[] =
    "in lowp vec4 fragmentColor;\n"
    "out lowp vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "	outColor = fragmentColor;\n"
    "}\n";

/*
================================================================================

ovrFramebuffer

================================================================================
*/

struct ovrFramebuffer {
    int Width;
    int Height;
    int TextureSwapChainLength;
    int TextureSwapChainIndex;
    ovrTextureSwapChain* ColorTextureSwapChain;
    GLuint* DepthBuffers;
    GLuint* FrameBuffers;
};

static void ovrFramebuffer_Clear(ovrFramebuffer* frameBuffer) {
    frameBuffer->Width = 0;
    frameBuffer->Height = 0;
    frameBuffer->TextureSwapChainLength = 0;
    frameBuffer->TextureSwapChainIndex = 0;
    frameBuffer->ColorTextureSwapChain = nullptr;
    frameBuffer->DepthBuffers = nullptr;
    frameBuffer->FrameBuffers = nullptr;
}

static bool ovrFramebuffer_Create(
    ovrFramebuffer* frameBuffer,
    const GLenum colorFormat,
    const int width,
    const int height) {
    frameBuffer->Width = width;
    frameBuffer->Height = height;

    frameBuffer->ColorTextureSwapChain = vrapi_CreateTextureSwapChain3(
        VRAPI_TEXTURE_TYPE_2D,
        colorFormat,
        width,
        height,
        1,
        3);
    frameBuffer->TextureSwapChainLength =
        vrapi_GetTextureSwapChainLength(frameBuffer->ColorTextureSwapChain);
    frameBuffer->DepthBuffers =
        (GLuint*)malloc(frameBuffer->TextureSwapChainLength * sizeof(GLuint));
    frameBuffer->FrameBuffers =
        (GLuint*)malloc(frameBuffer->TextureSwapChainLength * sizeof(GLuint));

    for (int i = 0; i < frameBuffer->TextureSwapChainLength; i++) {
        // Create the color buffer texture.
        const GLuint colorTexture =
            vrapi_GetTextureSwapChainHandle(frameBuffer->ColorTextureSwapChain, i);
        GLenum colorTextureTarget = GL_TEXTURE_2D;
        GL(glBindTexture(colorTextureTarget, colorTexture));
        GL(glTexParameteri(colorTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
        GL(glTexParameteri(colorTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
        GLfloat borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
        GL(glTexParameterfv(colorTextureTarget, GL_TEXTURE_BORDER_COLOR, borderColor));
        GL(glTexParameteri(colorTextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL(glTexParameteri(colorTextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL(glBindTexture(colorTextureTarget, 0));

                // Create depth buffer.
        GL(glGenRenderbuffers(1, &frameBuffer->DepthBuffers[i]));
        GL(glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer->DepthBuffers[i]));
        GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height));
        GL(glBindRenderbuffer(GL_RENDERBUFFER, 0));

                // Create the frame buffer.
        GL(glGenFramebuffers(1, &frameBuffer->FrameBuffers[i]));
        GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer->FrameBuffers[i]));
        GL(glFramebufferRenderbuffer(
            GL_DRAW_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            frameBuffer->DepthBuffers[i]));
        GL(glFramebufferTexture2D(
            GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0));
        GL(GLenum renderFramebufferStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
        GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
        if (renderFramebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
            ALOGE(
                "Incomplete frame buffer object: %s",
                GlFrameBufferStatusString(renderFramebufferStatus));
            return false;
        }
    }

    return true;
}

static void ovrFramebuffer_Destroy(ovrFramebuffer* frameBuffer) {
    GL(glDeleteFramebuffers(frameBuffer->TextureSwapChainLength, frameBuffer->FrameBuffers));
    GL(glDeleteRenderbuffers(frameBuffer->TextureSwapChainLength, frameBuffer->DepthBuffers));
    vrapi_DestroyTextureSwapChain(frameBuffer->ColorTextureSwapChain);

    free(frameBuffer->DepthBuffers);
    free(frameBuffer->FrameBuffers);

    ovrFramebuffer_Clear(frameBuffer);
}

static void ovrFramebuffer_SetCurrent(ovrFramebuffer* frameBuffer) {
    GL(glBindFramebuffer(
        GL_DRAW_FRAMEBUFFER, frameBuffer->FrameBuffers[frameBuffer->TextureSwapChainIndex]));
}

static void ovrFramebuffer_SetNone() {
    GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

static void ovrFramebuffer_Resolve(ovrFramebuffer* frameBuffer) {
    // Discard the depth buffer, so the tiler won't need to write it back out to memory.
    const GLenum depthAttachment[1] = {GL_DEPTH_ATTACHMENT};
    glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, depthAttachment);

    // We now let the resolve happen implicitly.
}

static void ovrFramebuffer_Advance(ovrFramebuffer* frameBuffer) {
    // Advance to the next texture from the set.
    frameBuffer->TextureSwapChainIndex =
        (frameBuffer->TextureSwapChainIndex + 1) % frameBuffer->TextureSwapChainLength;
}

/*
================================================================================

ovrScene

================================================================================
*/

#define NUM_INSTANCES 1500
#define NUM_ROTATIONS 16

struct ovrScene{
    bool CreatedScene;
    bool CreatedVAOs;
    unsigned int Random;
    ovrProgram Program;
    ovrGeometry Cube;
    GLuint SceneMatrices;
    GLuint InstanceTransformBuffer;
    ovrVector3f Rotations[NUM_ROTATIONS];
    ovrVector3f CubePositions[NUM_INSTANCES];
    int CubeRotations[NUM_INSTANCES];
};

static void ovrScene_Clear(ovrScene* scene) {
    scene->CreatedScene = false;
    scene->CreatedVAOs = false;
    scene->Random = 2;
    scene->SceneMatrices = 0;
    scene->InstanceTransformBuffer = 0;

    ovrProgram_Clear(&scene->Program);
    ovrGeometry_Clear(&scene->Cube);
}

static bool ovrScene_IsCreated(ovrScene* scene) {
    return scene->CreatedScene;
}

static void ovrScene_CreateVAOs(ovrScene* scene) {
    if (!scene->CreatedVAOs) {
        ovrGeometry_CreateVAO(&scene->Cube);

        // Modify the VAO to use the instance transform attributes.
        GL(glBindVertexArray(scene->Cube.VertexArrayObject));
        GL(glBindBuffer(GL_ARRAY_BUFFER, scene->InstanceTransformBuffer));
        for (int i = 0; i < 4; i++) {
            GL(glEnableVertexAttribArray(VERTEX_ATTRIBUTE_LOCATION_TRANSFORM + i));
            GL(glVertexAttribPointer(
                VERTEX_ATTRIBUTE_LOCATION_TRANSFORM + i,
                4,
                GL_FLOAT,
                false,
                4 * 4 * sizeof(float),
                (void*)(i * 4 * sizeof(float))));
            GL(glVertexAttribDivisor(VERTEX_ATTRIBUTE_LOCATION_TRANSFORM + i, 1));
        }
        GL(glBindVertexArray(0));

        scene->CreatedVAOs = true;
    }
}

static void ovrScene_DestroyVAOs(ovrScene* scene) {
    if (scene->CreatedVAOs) {
        ovrGeometry_DestroyVAO(&scene->Cube);

        scene->CreatedVAOs = false;
    }
}

// Returns a random float in the range [0, 1].
static float ovrScene_RandomFloat(ovrScene* scene) {
    scene->Random = 1664525L * scene->Random + 1013904223L;
    unsigned int rf = 0x3F800000 | (scene->Random & 0x007FFFFF);
    return (*(float*)&rf) - 1.0f;
}

static void ovrScene_Create(ovrScene* scene) {
    ovrProgram_Create(&scene->Program, VERTEX_SHADER, FRAGMENT_SHADER);
    ovrGeometry_CreateCube(&scene->Cube);

    // Create the instance transform attribute buffer.
    GL(glGenBuffers(1, &scene->InstanceTransformBuffer));
    GL(glBindBuffer(GL_ARRAY_BUFFER, scene->InstanceTransformBuffer));
    GL(glBufferData(GL_ARRAY_BUFFER, NUM_INSTANCES * 4 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW));
    GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    // Setup the scene matrices.
    GL(glGenBuffers(1, &scene->SceneMatrices));
    GL(glBindBuffer(GL_UNIFORM_BUFFER, scene->SceneMatrices));
    GL(glBufferData(
        GL_UNIFORM_BUFFER,
        2 * sizeof(ovrMatrix4f) /* 2 view matrices */ +
            2 * sizeof(ovrMatrix4f) /* 2 projection matrices */,
        nullptr,
        GL_STATIC_DRAW));
    GL(glBindBuffer(GL_UNIFORM_BUFFER, 0));

    // Setup random rotations.
    for (int i = 0; i < NUM_ROTATIONS; i++) {
        scene->Rotations[i].x = ovrScene_RandomFloat(scene);
        scene->Rotations[i].y = ovrScene_RandomFloat(scene);
        scene->Rotations[i].z = ovrScene_RandomFloat(scene);
    }

    // Setup random cube positions and rotations.
    for (int i = 0; i < NUM_INSTANCES; i++) {
        // Using volatile keeps the compiler from optimizing away multiple calls to
        // ovrScene_RandomFloat().
        volatile float rx, ry, rz;
        for (;;) {
            rx = (ovrScene_RandomFloat(scene) - 0.5f) * (50.0f + sqrt(NUM_INSTANCES));
            ry = (ovrScene_RandomFloat(scene) - 0.5f) * (50.0f + sqrt(NUM_INSTANCES));
            rz = (ovrScene_RandomFloat(scene) - 0.5f) * (50.0f + sqrt(NUM_INSTANCES));
            // If too close to 0,0,0
            if (fabsf(rx) < 4.0f && fabsf(ry) < 4.0f && fabsf(rz) < 4.0f) {
                continue;
            }
            // Test for overlap with any of the existing cubes.
            bool overlap = false;
            for (int j = 0; j < i; j++) {
                if (fabsf(rx - scene->CubePositions[j].x) < 4.0f &&
                    fabsf(ry - scene->CubePositions[j].y) < 4.0f &&
                    fabsf(rz - scene->CubePositions[j].z) < 4.0f) {
                    overlap = true;
                    break;
                }
            }
            if (!overlap) {
                break;
            }
        }

        rx *= 0.1f;
        ry *= 0.1f;
        rz *= 0.1f;

        // Insert into list sorted based on distance.
        int insert = 0;
        const float distSqr = rx * rx + ry * ry + rz * rz;
        for (int j = i; j > 0; j--) {
            const ovrVector3f* otherPos = &scene->CubePositions[j - 1];
            const float otherDistSqr =
                otherPos->x * otherPos->x + otherPos->y * otherPos->y + otherPos->z * otherPos->z;
            if (distSqr > otherDistSqr) {
                insert = j;
                break;
            }
            scene->CubePositions[j] = scene->CubePositions[j - 1];
            scene->CubeRotations[j] = scene->CubeRotations[j - 1];
        }

        scene->CubePositions[insert].x = rx;
        scene->CubePositions[insert].y = ry;
        scene->CubePositions[insert].z = rz;

        scene->CubeRotations[insert] = (int)(ovrScene_RandomFloat(scene) * (NUM_ROTATIONS - 0.1f));
    }

    scene->CreatedScene = true;

    ovrScene_CreateVAOs(scene);
}

static void ovrScene_Destroy(ovrScene* scene) {
    ovrScene_DestroyVAOs(scene);

    ovrProgram_Destroy(&scene->Program);
    ovrGeometry_Destroy(&scene->Cube);
    GL(glDeleteBuffers(1, &scene->InstanceTransformBuffer));
    GL(glDeleteBuffers(1, &scene->SceneMatrices));
    scene->CreatedScene = false;
}

/*
================================================================================

ovrSimulation

================================================================================
*/

struct ovrSimulation {
    ovrVector3f CurrentRotation;
};

static void ovrSimulation_Clear(ovrSimulation* simulation) {
    simulation->CurrentRotation.x = 0.0f;
    simulation->CurrentRotation.y = 0.0f;
    simulation->CurrentRotation.z = 0.0f;
}

static void ovrSimulation_Advance(ovrSimulation* simulation, double elapsedDisplayTime) {
    // Update rotation.
    simulation->CurrentRotation.x = (float)(elapsedDisplayTime);
    simulation->CurrentRotation.y = (float)(elapsedDisplayTime);
    simulation->CurrentRotation.z = (float)(elapsedDisplayTime);
}

/*
================================================================================

ovrRenderer

================================================================================
*/

struct ovrRenderer {
    ovrFramebuffer FrameBuffer[VRAPI_FRAME_LAYER_EYE_MAX];
    Renderer* rend = nullptr;
    int NumBuffers;
};

static void ovrRenderer_Clear(ovrRenderer* renderer) {
    for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++) {
        ovrFramebuffer_Clear(&renderer->FrameBuffer[eye]);
    }
    renderer->NumBuffers = VRAPI_FRAME_LAYER_EYE_MAX;
}

static void
ovrRenderer_Create(ovrRenderer* renderer, const ovrJava* java) {
    renderer->NumBuffers = VRAPI_FRAME_LAYER_EYE_MAX;

    // Create the frame buffers.
    for (int eye = 0; eye < renderer->NumBuffers; eye++) {
        ovrFramebuffer_Create(
            &renderer->FrameBuffer[eye],
            GL_RGBA8,
            vrapi_GetSystemPropertyInt(java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH),
            vrapi_GetSystemPropertyInt(java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT));
    }
    renderer->rend = CreateRenderer();
    renderer->rend->Init();
    renderer->rend->SetWindowSize(renderer->FrameBuffer[0].Width, renderer->FrameBuffer[0].Height);
}

static void ovrRenderer_Destroy(ovrRenderer* renderer) {
    for (int eye = 0; eye < renderer->NumBuffers; eye++) {
        ovrFramebuffer_Destroy(&renderer->FrameBuffer[eye]);
    }
}

static ovrLayerProjection2 ovrRenderer_RenderFrame(
    ovrRenderer* renderer,
    const ovrJava* java,
    const ovrScene* scene,
    const ovrSimulation* simulation,
    const ovrTracking2* tracking,
    ovrMobile* ovr) {
    ovrMatrix4f rotationMatrices[NUM_ROTATIONS];
    for (int i = 0; i < NUM_ROTATIONS; i++) {
        rotationMatrices[i] = ovrMatrix4f_CreateRotation(
            scene->Rotations[i].x * simulation->CurrentRotation.x,
            scene->Rotations[i].y * simulation->CurrentRotation.y,
            scene->Rotations[i].z * simulation->CurrentRotation.z);
    }

    // Update the instance transform attributes.
    GL(glBindBuffer(GL_ARRAY_BUFFER, scene->InstanceTransformBuffer));
    GL(ovrMatrix4f* cubeTransforms = (ovrMatrix4f*)glMapBufferRange(
           GL_ARRAY_BUFFER,
           0,
           NUM_INSTANCES * sizeof(ovrMatrix4f),
           GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
    for (int i = 0; i < NUM_INSTANCES; i++) {
        const int index = scene->CubeRotations[i];

        // Write in order in case the mapped buffer lives on write-combined memory.
        cubeTransforms[i].M[0][0] = rotationMatrices[index].M[0][0];
        cubeTransforms[i].M[0][1] = rotationMatrices[index].M[0][1];
        cubeTransforms[i].M[0][2] = rotationMatrices[index].M[0][2];
        cubeTransforms[i].M[0][3] = rotationMatrices[index].M[0][3];

        cubeTransforms[i].M[1][0] = rotationMatrices[index].M[1][0];
        cubeTransforms[i].M[1][1] = rotationMatrices[index].M[1][1];
        cubeTransforms[i].M[1][2] = rotationMatrices[index].M[1][2];
        cubeTransforms[i].M[1][3] = rotationMatrices[index].M[1][3];

        cubeTransforms[i].M[2][0] = rotationMatrices[index].M[2][0];
        cubeTransforms[i].M[2][1] = rotationMatrices[index].M[2][1];
        cubeTransforms[i].M[2][2] = rotationMatrices[index].M[2][2];
        cubeTransforms[i].M[2][3] = rotationMatrices[index].M[2][3];

        cubeTransforms[i].M[3][0] = scene->CubePositions[i].x;
        cubeTransforms[i].M[3][1] = scene->CubePositions[i].y;
        cubeTransforms[i].M[3][2] = scene->CubePositions[i].z;
        cubeTransforms[i].M[3][3] = 1.0f;
    }
    GL(glUnmapBuffer(GL_ARRAY_BUFFER));
    GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    ovrTracking2 updatedTracking = *tracking;

    ovrMatrix4f eyeViewMatrixTransposed[2];
    eyeViewMatrixTransposed[0] = ovrMatrix4f_Transpose(&updatedTracking.Eye[0].ViewMatrix);
    eyeViewMatrixTransposed[1] = ovrMatrix4f_Transpose(&updatedTracking.Eye[1].ViewMatrix);

    ovrMatrix4f projectionMatrixTransposed[2];
    projectionMatrixTransposed[0] = ovrMatrix4f_Transpose(&updatedTracking.Eye[0].ProjectionMatrix);
    projectionMatrixTransposed[1] = ovrMatrix4f_Transpose(&updatedTracking.Eye[1].ProjectionMatrix);

    // Update the scene matrices.
    GL(glBindBuffer(GL_UNIFORM_BUFFER, scene->SceneMatrices));
    GL(ovrMatrix4f* sceneMatrices = (ovrMatrix4f*)glMapBufferRange(
           GL_UNIFORM_BUFFER,
           0,
           2 * sizeof(ovrMatrix4f) /* 2 view matrices */ +
               2 * sizeof(ovrMatrix4f) /* 2 projection matrices */,
           GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

    if (sceneMatrices != nullptr) {
        memcpy((char*)sceneMatrices, &eyeViewMatrixTransposed, 2 * sizeof(ovrMatrix4f));
        memcpy(
            (char*)sceneMatrices + 2 * sizeof(ovrMatrix4f),
            &projectionMatrixTransposed,
            2 * sizeof(ovrMatrix4f));
    }

    GL(glUnmapBuffer(GL_UNIFORM_BUFFER));
    GL(glBindBuffer(GL_UNIFORM_BUFFER, 0));

    ovrLayerProjection2 layer = vrapi_DefaultLayerProjection2();
    layer.HeadPose = updatedTracking.HeadPose;
    for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++) {
        ovrFramebuffer* frameBuffer = &renderer->FrameBuffer[renderer->NumBuffers == 1 ? 0 : eye];
        layer.Textures[eye].ColorSwapChain = frameBuffer->ColorTextureSwapChain;
        layer.Textures[eye].SwapChainIndex = frameBuffer->TextureSwapChainIndex;
        layer.Textures[eye].TexCoordsFrom =
            ovrMatrix4f_TanAngleMatrixFromProjection(&updatedTracking.Eye[eye].ProjectionMatrix);
    }
    layer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;

    // Render the eye images.
    for (int eye = 0; eye < renderer->NumBuffers; eye++) {
        // NOTE: In the non-mv case, latency can be further reduced by updating the sensor
        // prediction for each eye (updates orientation, not position)
        ovrFramebuffer* frameBuffer = &renderer->FrameBuffer[eye];
        ovrFramebuffer_SetCurrent(frameBuffer);

        if (false) {
            GL(glUseProgram(scene->Program.Program));
            GL(glBindBufferBase(
                GL_UNIFORM_BUFFER,
                scene->Program.UniformBinding[ovrUniform::Index::SCENE_MATRICES],
                scene->SceneMatrices));
            if (scene->Program.UniformLocation[ovrUniform::Index::VIEW_ID] >=
            0) // NOTE: will not be present when multiview path is enabled.
            {
                GL(glUniform1i(scene->Program.UniformLocation[ovrUniform::Index::VIEW_ID], eye));
            }
            GL(glEnable(GL_SCISSOR_TEST));
            GL(glDepthMask(GL_TRUE));
            GL(glEnable(GL_DEPTH_TEST));
            GL(glDepthFunc(GL_LEQUAL));
            GL(glEnable(GL_CULL_FACE));
            GL(glCullFace(GL_BACK));
            GL(glViewport(0, 0, frameBuffer->Width, frameBuffer->Height));
            GL(glScissor(0, 0, frameBuffer->Width, frameBuffer->Height));
            GL(glClearColor(0.125f, 0.0f, 0.125f, 1.0f));
            GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
            GL(glBindVertexArray(scene->Cube.VertexArrayObject));
            GL(glDrawElementsInstanced(
                GL_TRIANGLES, scene->Cube.IndexCount, GL_UNSIGNED_SHORT, nullptr, NUM_INSTANCES));
            GL(glBindVertexArray(0));
            GL(glUseProgram(0));

        }
        ovrFramebuffer_Resolve(frameBuffer);
        ovrFramebuffer_Advance(frameBuffer);

        if (true){
            renderer->rend->camFrustum = ToR3(projectionMatrixTransposed[eye]);
            renderer->rend->camPose    = ToR3(eyeViewMatrixTransposed[eye]).Inverted();
            renderer->rend->Draw();
        }
    }

    ovrFramebuffer_SetNone();

    return layer;
}


/*
================================================================================

ovrApp

================================================================================
*/

struct ovrApp {
    ovrJava Java;
    ovrEgl Egl;
    ANativeWindow* NativeWindow;
    bool Resumed;
    ovrMobile* Ovr;
    ovrScene Scene;
    ovrSimulation Simulation;
    long long FrameIndex;
    double DisplayTime;
    int SwapInterval;
    int CpuLevel;
    int GpuLevel;
    int MainThreadTid;
    int RenderThreadTid;
    bool BackButtonDownLastFrame;
    ovrRenderer Renderer;
    ovrDeviceID left;
    ovrDeviceID right;
};

static void ovrApp_Clear(ovrApp* app) {
    app->Java.Vm = nullptr;
    app->Java.Env = nullptr;
    app->Java.ActivityObject = nullptr;
    app->NativeWindow = nullptr;
    app->Resumed = false;
    app->Ovr = nullptr;
    app->FrameIndex = 1;
    app->DisplayTime = 0;
    app->SwapInterval = 1;
    app->CpuLevel = 2;
    app->GpuLevel = 2;
    app->MainThreadTid = 0;
    app->RenderThreadTid = 0;
    app->BackButtonDownLastFrame = false;

    ovrEgl_Clear(&app->Egl);
    ovrScene_Clear(&app->Scene);
    ovrSimulation_Clear(&app->Simulation);
    ovrRenderer_Clear(&app->Renderer);
}

static void ovrApp_HandleVrModeChanges(ovrApp* app) {
    if (app->Resumed != false && app->NativeWindow != nullptr) {
        if (app->Ovr == nullptr) {
            ovrModeParms parms = vrapi_DefaultModeParms(&app->Java);
            // No need to reset the FLAG_FULLSCREEN window flag when using a View
            parms.Flags &= ~VRAPI_MODE_FLAG_RESET_WINDOW_FULLSCREEN;

            parms.Flags |= VRAPI_MODE_FLAG_NATIVE_WINDOW;
            parms.Display = (size_t)app->Egl.Display;
            parms.WindowSurface = (size_t)app->NativeWindow;
            parms.ShareContext = (size_t)app->Egl.Context;

            ALOGV("        eglGetCurrentSurface( EGL_DRAW ) = %p", eglGetCurrentSurface(EGL_DRAW));

            ALOGV("        vrapi_EnterVrMode()");

            app->Ovr = vrapi_EnterVrMode(&parms);
            /*app->left = ovrDeviceIdType_Invalid;
            app->right = ovrDeviceIdType_Invalid;
            for (ovrDeviceID deviceIndex = 0;; deviceIndex++) { // Come Back Here!
                ovrInputCapabilityHeader capsHeader;

                if (vrapi_EnumerateInputDevices(app->Ovr, deviceIndex, &capsHeader) < 0) {
                    break; // no more devices
                }

                /// deviceID is not the same as deviceIndex!
                ovrDeviceID deviceID = capsHeader.DeviceID;
                if (deviceID == ovrDeviceIdType_Invalid) {
                    ALOGV("HandleVRInputEvents - Invalid deviceID for deviceIndex=%u", deviceIndex);
                    assert(deviceID != ovrDeviceIdType_Invalid);
                    continue;
                }

                // Focus on remotes for now
                if (capsHeader.Type == ovrControllerType_TrackedRemote) {
                    ovrInputTrackedRemoteCapabilities remoteCaps;
                    remoteCaps.Header = capsHeader;
                    if (ovrSuccess == vrapi_GetInputDeviceCapabilities(app->Ovr, &remoteCaps.Header)) {
                        bool isLeft = (remoteCaps.ControllerCapabilities & ovrControllerCaps_LeftHand) != 0;
                        bool isRight = (remoteCaps.ControllerCapabilities & ovrControllerCaps_RightHand) != 0;
                        // allow for single-handed controlles

                        if (isLeft) {
                            app->left = deviceID;
                        }
                        if (isRight) {
                            app->right = deviceID;
                        }
                    }
                }
            }*/
            vrapi_SetTrackingSpace(app->Ovr, VRAPI_TRACKING_SPACE_STAGE);

            ALOGV("        eglGetCurrentSurface( EGL_DRAW ) = %p", eglGetCurrentSurface(EGL_DRAW));

            // If entering VR mode failed then the ANativeWindow was not valid.
            if (app->Ovr == nullptr) {
                ALOGE("Invalid ANativeWindow!");
                app->NativeWindow = nullptr;
            }

            // Set performance parameters once we have entered VR mode and have a valid ovrMobile.
            if (app->Ovr != nullptr) {
                vrapi_SetClockLevels(app->Ovr, app->CpuLevel, app->GpuLevel);

                ALOGV("		vrapi_SetClockLevels( %d, %d )", app->CpuLevel, app->GpuLevel);

                vrapi_SetPerfThread(app->Ovr, VRAPI_PERF_THREAD_TYPE_MAIN, app->MainThreadTid);

                ALOGV("		vrapi_SetPerfThread( MAIN, %d )", app->MainThreadTid);

                vrapi_SetPerfThread(
                    app->Ovr, VRAPI_PERF_THREAD_TYPE_RENDERER, app->RenderThreadTid);

                ALOGV("		vrapi_SetPerfThread( RENDERER, %d )", app->RenderThreadTid);
            }
        }
    } else {
        if (app->Ovr != nullptr) {
            ALOGV("        eglGetCurrentSurface( EGL_DRAW ) = %p", eglGetCurrentSurface(EGL_DRAW));

            ALOGV("        vrapi_LeaveVrMode()");

            vrapi_LeaveVrMode(app->Ovr);
            app->Ovr = nullptr;

            ALOGV("        eglGetCurrentSurface( EGL_DRAW ) = %p", eglGetCurrentSurface(EGL_DRAW));
        }
    }
}

static void ovrApp_HandleInput(ovrApp* app) {
    bool backButtonDownThisFrame = false;

    for (int i = 0;; i++) {
        ovrInputCapabilityHeader cap;
        ovrResult result = vrapi_EnumerateInputDevices(app->Ovr, i, &cap);
        if (result < 0) {
            break;
        }

        if (cap.Type == ovrControllerType_TrackedRemote) {
            ovrInputStateTrackedRemote trackedRemoteState;
            trackedRemoteState.Header.ControllerType = ovrControllerType_TrackedRemote;
            result = vrapi_GetCurrentInputState(app->Ovr, cap.DeviceID, &trackedRemoteState.Header);
            if (result == ovrSuccess) {
                backButtonDownThisFrame |= trackedRemoteState.Buttons & ovrButton_Back;
                backButtonDownThisFrame |= trackedRemoteState.Buttons & ovrButton_B;
                backButtonDownThisFrame |= trackedRemoteState.Buttons & ovrButton_Y;
            }
        }
    }

    bool backButtonDownLastFrame = app->BackButtonDownLastFrame;
    app->BackButtonDownLastFrame = backButtonDownThisFrame;

    if (backButtonDownLastFrame && !backButtonDownThisFrame) {
        ALOGV("back button short press");
        ALOGV("        vrapi_ShowSystemUI( confirmQuit )");
        vrapi_ShowSystemUI(&app->Java, VRAPI_SYS_UI_CONFIRM_QUIT_MENU);
    }
}

static void ovrApp_HandleVrApiEvents(ovrApp* app) {
    ovrEventDataBuffer eventDataBuffer = {};

    // Poll for VrApi events
    for (;;) {
        ovrEventHeader* eventHeader = (ovrEventHeader*)(&eventDataBuffer);
        ovrResult res = vrapi_PollEvent(eventHeader);
        if (res != ovrSuccess) {
            break;
        }

        switch (eventHeader->EventType) {
            case VRAPI_EVENT_DATA_LOST:
                ALOGV("vrapi_PollEvent: Received VRAPI_EVENT_DATA_LOST");
                break;
            case VRAPI_EVENT_VISIBILITY_GAINED:
                ALOGV("vrapi_PollEvent: Received VRAPI_EVENT_VISIBILITY_GAINED");
                break;
            case VRAPI_EVENT_VISIBILITY_LOST:
                ALOGV("vrapi_PollEvent: Received VRAPI_EVENT_VISIBILITY_LOST");
                break;
            case VRAPI_EVENT_FOCUS_GAINED:
                // FOCUS_GAINED is sent when the application is in the foreground and has
                // input focus. This may be due to a system overlay relinquishing focus
                // back to the application.
                ALOGV("vrapi_PollEvent: Received VRAPI_EVENT_FOCUS_GAINED");
                break;
            case VRAPI_EVENT_FOCUS_LOST:
                // FOCUS_LOST is sent when the application is no longer in the foreground and
                // therefore does not have input focus. This may be due to a system overlay taking
                // focus from the application. The application should take appropriate action when
                // this occurs.
                ALOGV("vrapi_PollEvent: Received VRAPI_EVENT_FOCUS_LOST");
                break;
            default:
                ALOGV("vrapi_PollEvent: Unknown event");
                break;
        }
    }
}

/*
================================================================================

Native Activity

================================================================================
*/

/**
 * Process the next main command.
 */
static void app_handle_cmd(struct android_app* app, int32_t cmd) {
    ovrApp* appState = (ovrApp*)app->userData;

    switch (cmd) {
        // There is no APP_CMD_CREATE. The ANativeActivity creates the
        // application thread from onCreate(). The application thread
        // then calls android_main().
        case APP_CMD_START: {
            ALOGV("onStart()");
            ALOGV("    APP_CMD_START");
            break;
        }
        case APP_CMD_RESUME: {
            ALOGV("onResume()");
            ALOGV("    APP_CMD_RESUME");
            appState->Resumed = true;
            break;
        }
        case APP_CMD_PAUSE: {
            ALOGV("onPause()");
            ALOGV("    APP_CMD_PAUSE");
            appState->Resumed = false;
            break;
        }
        case APP_CMD_STOP: {
            ALOGV("onStop()");
            ALOGV("    APP_CMD_STOP");
            break;
        }
        case APP_CMD_DESTROY: {
            ALOGV("onDestroy()");
            ALOGV("    APP_CMD_DESTROY");
            appState->NativeWindow = nullptr;
            break;
        }
        case APP_CMD_INIT_WINDOW: {
            ALOGV("surfaceCreated()");
            ALOGV("    APP_CMD_INIT_WINDOW");
            appState->NativeWindow = app->window;
            break;
        }
        case APP_CMD_TERM_WINDOW: {
            ALOGV("surfaceDestroyed()");
            ALOGV("    APP_CMD_TERM_WINDOW");
            appState->NativeWindow = nullptr;
            break;
        }
    }
}

void copy_assets(AAssetManager* mgr) {
    AAssetDir* assetDir = AAssetManager_openDir(mgr, "");
    const char* filename = (const char*)NULL;
    static const int SZ = 1 << 12;
    char* buf = new char[SZ];
    while ((filename = AAssetDir_getNextFileName(assetDir)) != NULL) {
        AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_STREAMING);
        int nb_read = 0;
        char fullPath[512];
        strcpy(fullPath, PATH_PREFIX);
        strcat(fullPath, filename);
        FILE* out = fopen(fullPath, "w");
        ALOGV("writing file: %s", fullPath);
        int total_bytes_written = 0;
        while ((nb_read = AAsset_read(asset, buf, SZ)) > 0) {
            fwrite(buf, nb_read, 1, out);
            total_bytes_written += nb_read;
        }
        fclose(out);
        AAsset_close(asset);
        ALOGV("wrote file: %s, %d bytes", fullPath, total_bytes_written);
    }
    delete[] buf;
    AAssetDir_close(assetDir);
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */

void android_main(struct android_app* app) {
    ALOGV("----------------------------------------------------------------");
    ALOGV("android_app_entry()");
    ALOGV("    android_main()");

    ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);

    ovrJava java;
    java.Vm = app->activity->vm;
    java.Vm->AttachCurrentThread(&java.Env, nullptr);
    java.ActivityObject = app->activity->clazz;

    // Note that AttachCurrentThread will reset the thread name.
    prctl(PR_SET_NAME, (long)"OVR::Main", 0, 0, 0);

    copy_assets(app->activity->assetManager);

    const ovrInitParms initParms = vrapi_DefaultInitParms(&java);
    int32_t initResult = vrapi_Initialize(&initParms);
    if (initResult != VRAPI_INITIALIZE_SUCCESS) {
        // If intialization failed, vrapi_* function calls will not be available.
        exit(0);
    }

    ovrApp appState;
    ovrApp_Clear(&appState);
    appState.Java = java;

    ovrEgl_CreateContext(&appState.Egl, nullptr);

    EglInitExtensions();

    appState.CpuLevel = CPU_LEVEL;
    appState.GpuLevel = GPU_LEVEL;
    appState.MainThreadTid = gettid();

    ovrRenderer_Create(&appState.Renderer, &java);

    app->userData = &appState;
    app->onAppCmd = app_handle_cmd;

    const double startTime = GetTimeInSeconds();

    int lTrHeld = 0;
    int rTrHeld = 0;
    bool buttonAHeld = false;
    bool buttonXHeld = false;
    bool teleportMode = true;
    bool tpHit = false;

    float worldScale = 2.0;
    Vec3f headPos;
    Vec3f position;
    float distance = 1.0;
    float initDist = 1.0;

    while (app->destroyRequested == 0) {
        // Read all pending events.
        for (;;) {
            int events;
            struct android_poll_source* source;
            const int timeoutMilliseconds =
                (appState.Ovr == nullptr && app->destroyRequested == 0) ? -1 : 0;
            if (ALooper_pollAll(timeoutMilliseconds, nullptr, &events, (void**)&source) < 0) {
                break;
            }

            // Process this event.
            if (source != nullptr) {
                source->process(app, source);
            }

            ovrApp_HandleVrModeChanges(&appState);
        }

        // We must read from the event queue with regular frequency.
        ovrApp_HandleVrApiEvents(&appState);

        ovrApp_HandleInput(&appState);

        if (appState.Ovr == nullptr) {
            continue;
        }

        // Create the scene if not yet created.
        // The scene is created here to be able to show a loading icon.
        if (!ovrScene_IsCreated(&appState.Scene)) {
            // Show a loading icon.
            int frameFlags = 0;
            frameFlags |= VRAPI_FRAME_FLAG_FLUSH;

            ovrLayerProjection2 blackLayer = vrapi_DefaultLayerBlackProjection2();
            blackLayer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_INHIBIT_SRGB_FRAMEBUFFER;

            ovrLayerLoadingIcon2 iconLayer = vrapi_DefaultLayerLoadingIcon2();
            iconLayer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_INHIBIT_SRGB_FRAMEBUFFER;

            const ovrLayerHeader2* layers[] = {
                &blackLayer.Header,
                &iconLayer.Header,
            };

            ovrSubmitFrameDescription2 frameDesc = {0};
            frameDesc.Flags = frameFlags;
            frameDesc.SwapInterval = 1;
            frameDesc.FrameIndex = appState.FrameIndex;
            frameDesc.DisplayTime = appState.DisplayTime;
            frameDesc.LayerCount = 2;
            frameDesc.Layers = layers;

            vrapi_SubmitFrame2(appState.Ovr, &frameDesc);

            // Create the scene.
            ovrScene_Create(&appState.Scene);
        }

        // This is the only place the frame index is incremented, right before
        // calling vrapi_GetPredictedDisplayTime().
        appState.FrameIndex++;

        // Get the HMD pose, predicted for the middle of the time period during which
        // the new eye images will be displayed. The number of frames predicted ahead
        // depends on the pipeline depth of the engine and the synthesis rate.
        // The better the prediction, the less black will be pulled in at the edges.
        const double predictedDisplayTime =
            vrapi_GetPredictedDisplayTime(appState.Ovr, appState.FrameIndex);
        const ovrTracking2 tracking =
            vrapi_GetPredictedTracking2(appState.Ovr, predictedDisplayTime);

        headPos = ToR3(tracking.HeadPose.Pose.Position);
        appState.DisplayTime = predictedDisplayTime;

        // Advance the simulation based on the elapsed time since start of loop till predicted
        // display time.
        ovrSimulation_Advance(&appState.Simulation, predictedDisplayTime - startTime);

        // Render eye images and setup the primary layer using ovrTracking2.

        //// Get controller input states and positions

        appState.left = ovrDeviceIdType_Invalid;
        appState.right = ovrDeviceIdType_Invalid;

        for (ovrDeviceID deviceIndex = 0;; deviceIndex++) { // Come Back Here!
            ovrInputCapabilityHeader capsHeader;

            if (vrapi_EnumerateInputDevices(appState.Ovr, deviceIndex, &capsHeader) < 0) {
                break; // no more devices
            }

                /// deviceID is not the same as deviceIndex!
            ovrDeviceID deviceID = capsHeader.DeviceID;
            if (deviceID == ovrDeviceIdType_Invalid) {
                ALOGV("HandleVRInputEvents - Invalid deviceID for deviceIndex=%u", deviceIndex);
                assert(deviceID != ovrDeviceIdType_Invalid);
                continue;
            }

                // Focus on remotes for now
            if (capsHeader.Type == ovrControllerType_TrackedRemote) {
                ovrInputTrackedRemoteCapabilities remoteCaps;
                remoteCaps.Header = capsHeader;
                if (ovrSuccess == vrapi_GetInputDeviceCapabilities(appState.Ovr, &remoteCaps.Header)) {
                    bool isLeft = (remoteCaps.ControllerCapabilities & ovrControllerCaps_LeftHand) != 0;
                    bool isRight = (remoteCaps.ControllerCapabilities & ovrControllerCaps_RightHand) != 0;

                    if (isLeft) {
                        appState.left = deviceID;
                    }
                    if (isRight) {
                        appState.right = deviceID;
                    }
                }
            }
        }

        appState.Renderer.rend->headPoseInTracking = ToR3(tracking.HeadPose.Pose);

        ovrTracking remoteTrackingL;
        ovrInputStateTrackedRemote stateL;
        ovrTracking remoteTrackingR;
        ovrInputStateTrackedRemote stateR;
        if (appState.left != ovrDeviceIdType_Invalid){
            vrapi_GetInputTrackingState(appState.Ovr, appState.left, appState.DisplayTime, &remoteTrackingL);
            appState.Renderer.rend->leftPose = ToR3(remoteTrackingL.HeadPose.Pose);
            stateL.Header.ControllerType = ovrControllerType_TrackedRemote;
            vrapi_GetCurrentInputState(appState.Ovr, appState.left, &stateL.Header);
        } if (appState.right != ovrDeviceIdType_Invalid){
            vrapi_GetInputTrackingState(appState.Ovr, appState.right, appState.DisplayTime, &remoteTrackingR);
            appState.Renderer.rend->rightPose = ToR3(remoteTrackingR.HeadPose.Pose);
            stateR.Header.ControllerType = ovrControllerType_TrackedRemote;
            vrapi_GetCurrentInputState(appState.Ovr, appState.right, &stateR.Header);
        }

        //// Scaling math
        if (stateL.GripTrigger >= 0.95 && stateR.GripTrigger >= 0.95) {
            const Vec3f& lPos = appState.Renderer.rend->leftPose.t;
            const Vec3f& rPos = appState.Renderer.rend->rightPose.t;
            float dist = (lPos - rPos).Length();
            if (initDist == 1.0) {
                initDist = dist;
            }
            distance = dist;
            Vec3f scaleOriginInTracking( headPos.x, 0, headPos.z);
            appState.Renderer.rend->SetScale(worldScale * (distance/initDist), scaleOriginInTracking);
        } else {
            worldScale *= distance/initDist;
            initDist = 1.0;
            distance = 1.0;
        }
        

        //// Trigger Pointing

        Vec3f rotation;

        if (stateL.IndexTrigger >= 0.95) {
            Posef worldFromLc = appState.Renderer.rend->leftPose;
            Vec3f nIW3 = worldFromLc * Vec3f(0,0,0);
            Vec3f fIW3 = worldFromLc * Vec3f(0,0,-100);
            if (true) {
                fIW3 *= 100;

                appState.Renderer.rend->RayInWorld(nIW3, fIW3);
                if (lTrHeld == 0) {
                    appState.Renderer.rend->Intersect(nIW3, fIW3);
                } else if (appState.Renderer.rend->intersect) {
                    Vec3f i;

                    Linef line(nIW3, fIW3);
                    Planef plane(Vec3f(0, 1, 0), appState.Renderer.rend->intLoc.y);
                    plane.Intersect(line, i);
                    appState.Renderer.rend->Drag(i);
                }
            }
            lTrHeld += 1;
        } else {
            lTrHeld = 0;
        }
        
        if (stateR.IndexTrigger >= 0.95) {
            Posef worldFromRc = appState.Renderer.rend->rightPose;
            Vec3f nIW3 = worldFromRc * Vec3f(0,0,0);
            Vec3f fIW3 = worldFromRc * Vec3f(0,0,-100);
            if (teleportMode){
                position = nIW3;
                fIW3 = (fIW3 / worldScale) / 10;
                fIW3.y *= 1.5;

                tpHit = appState.Renderer.rend->BalisticProj(position, fIW3);
            } else {
                fIW3 *= 100;

                appState.Renderer.rend->RayInWorld(nIW3, fIW3);
                if (rTrHeld == 0) {
                    appState.Renderer.rend->Intersect(nIW3, fIW3);
                } else if (appState.Renderer.rend->intersect) {
                    Vec3f i;

                    Linef line(nIW3, fIW3);
                    Planef plane(Vec3f(0, 1, 0), appState.Renderer.rend->intLoc.y);
                    plane.Intersect(line, i);
                    appState.Renderer.rend->Drag(i);
                }
            }
            rTrHeld += 1;
        } else {
            if (tpHit) {
                appState.Renderer.rend->TeleportInApp(position);
                tpHit = false;
            }
            rTrHeld = 0;
        }

        //// Buttons

        if (stateR.Buttons & ovrTouch_A) {
            if (!buttonAHeld) {
                teleportMode = !teleportMode;
                //appState.Renderer.rend->drawCenterAxis = !appState.Renderer.rend->drawCenterAxis;
                buttonAHeld = true; 
            }
        } else {
            buttonAHeld = false;
        }

        ////

        const ovrLayerProjection2 worldLayer = ovrRenderer_RenderFrame(
            &appState.Renderer,
            &appState.Java,
            &appState.Scene,
            &appState.Simulation,
            &tracking,
            appState.Ovr);

        const ovrLayerHeader2* layers[] = {&worldLayer.Header};

        ovrSubmitFrameDescription2 frameDesc = {0};
        frameDesc.Flags = 0;
        frameDesc.SwapInterval = appState.SwapInterval;
        frameDesc.FrameIndex = appState.FrameIndex;
        frameDesc.DisplayTime = appState.DisplayTime;
        frameDesc.LayerCount = 1;
        frameDesc.Layers = layers;

        // Hand over the eye images to the time warp.
        vrapi_SubmitFrame2(appState.Ovr, &frameDesc);
    }

    ovrRenderer_Destroy(&appState.Renderer);

    ovrScene_Destroy(&appState.Scene);
    ovrEgl_DestroyContext(&appState.Egl);

    vrapi_Shutdown();

    java.Vm->DetachCurrentThread();
}
