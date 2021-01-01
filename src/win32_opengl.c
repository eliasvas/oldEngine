//Here, OpenGL functions are loaded, it's really
//nothing you should worry about, platform.h
//is your OS as far as you are concerned!
#include "platform.h"
#include <windows.h>
#include <gl/gl.h>
#include "ext/glext.h"
#include "ext/wglext.h"
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "tools.h"

#pragma comment (lib, "opengl32.lib")

/* In short: We get a Device Context with GetDC(),
 * we pick a hardware accelerated pixel format with ChoosePixelFormat(),
 * and associate it with a Device Context via SetPixelFormat(),
 * then we create a Rendering Context with wglCreateContext() and
 * set it as current with wglMakeCurrent()*/

/*The problem is ChoosePixelFormat() and SetPixelFormat() are not accesible 
 * so you can't enable multisampling, floating-point framebuffers etc.
 * So we need two new functions, wglChoosePixelFormatARB() and wglCreateContextAttribsARB()
 * but to get their function pointers.. we need a Rendering Context! So we have to make a 
 * fake one first and obtain the function pointers [and then destroy it and make a proper one!]*/

static HGLRC win32_opengl_context;

PFNWGLCHOOSEPIXELFORMATARBPROC     wglChoosePixelFormatARB;
PFNWGLCREATECONTEXTATTRIBSARBPROC  wglCreateContextAttribsARB;
PFNWGLMAKECONTEXTCURRENTARBPROC    wglMakeContextCurrentARB;
PFNWGLSWAPINTERVALEXTPROC          wglSwapIntervalEXT;

#define GLProc(type, name) PFNGL##type##PROC name;

GLProc( GENBUFFERS, glGenBuffers);
GLProc( BINDBUFFER, glBindBuffer);
GLProc( DRAWBUFFERS, glDrawBuffers);
GLProc( USEPROGRAM, glUseProgram);
GLProc( SHADERSOURCE, glShaderSource);
GLProc( COMPILESHADER, glCompileShader);
GLProc( GETSHADERIV, glGetShaderiv);
GLProc( MAPBUFFERRANGE, glMapBufferRange);
GLProc( MAPBUFFER, glMapBuffer);
GLProc( CREATESHADER, glCreateShader);
GLProc( GETSHADERINFOLOG, glGetShaderInfoLog);
GLProc( GETPROGRAMINFOLOG, glGetProgramInfoLog);
GLProc( CREATEPROGRAM, glCreateProgram);
GLProc( ATTACHSHADER, glAttachShader);
GLProc( DELETESHADER, glDeleteShader);
GLProc( DELETEPROGRAM, glDeleteProgram);
GLProc( LINKPROGRAM, glLinkProgram);
GLProc( GETPROGRAMIV, glGetProgramiv);
GLProc( UNIFORM1I, glUniform1i);
GLProc( UNIFORM3F, glUniform3f);
GLProc( UNIFORM1IV, glUniform1iv);
GLProc( UNIFORM2FV, glUniform2fv);
GLProc( UNIFORM1F, glUniform1f);
GLProc( ACTIVETEXTURE, glActiveTexture);
GLProc( VERTEXATTRIBDIVISOR, glVertexAttribDivisor);
GLProc( GETUNIFORMLOCATION, glGetUniformLocation);
GLProc( GENVERTEXARRAYS, glGenVertexArrays);
GLProc( DRAWELEMENTSINSTANCED, glDrawElementsInstanced);
GLProc( DRAWARRAYSINSTANCED, glDrawArraysInstanced);
GLProc( BINDVERTEXARRAY, glBindVertexArray);
GLProc( UNIFORMMATRIX4FV, glUniformMatrix4fv);
GLProc( BUFFERDATA, glBufferData);
GLProc( VERTEXATTRIBPOINTER, glVertexAttribPointer);
GLProc( VERTEXATTRIBIPOINTER, glVertexAttribIPointer);
GLProc( ENABLEVERTEXATTRIBARRAY, glEnableVertexAttribArray);
GLProc( GENERATEMIPMAP, glGenerateMipmap);
GLProc( GENFRAMEBUFFERS, glGenFramebuffers);
GLProc( FRAMEBUFFERTEXTURE2D, glFramebufferTexture2D);
GLProc( BINDFRAMEBUFFER, glBindFramebuffer);
GLProc( CHECKFRAMEBUFFERSTATUS, glCheckFramebufferStatus);
GLProc( BINDRENDERBUFFER, glBindRenderbuffer);
GLProc( RENDERBUFFERSTORAGE, glRenderbufferStorage);
GLProc( GENRENDERBUFFERS, glGenRenderbuffers);
GLProc( FRAMEBUFFERRENDERBUFFER, glFramebufferRenderbuffer);
GLProc( TEXIMAGE3D, glTexImage3D);
GLProc( BINDIMAGETEXTURE, glBindImageTexture);
GLProc( MEMORYBARRIER, glMemoryBarrier);
GLProc( COPYIMAGESUBDATA, glCopyImageSubData);
GLProc( BLENDFUNCSEPARATE, glBlendFuncSeparate);
GLProc( DELETEFRAMEBUFFERS, glDeleteFramebuffers);
GLProc( BLITFRAMEBUFFER, glBlitFramebuffer);


static void *GetGLFuncAddress(const char *name)
{
  void *p = (void *)wglGetProcAddress(name);
  if(p == 0 ||
    (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
    (p == (void*)-1))
  {
    HMODULE module = LoadLibraryA("opengl32.dll");
    p = (void *)GetProcAddress(module, name);
  }

  return p;
}

//NOTE(ilias): maybe this should happen in fake window stage?
static void 
LoadAllOpenGLProcedures()
{
   glGenBuffers = (PFNGLGENBUFFERSPROC)GetGLFuncAddress("glGenBuffers"); 
   glBindBuffer = (PFNGLBINDBUFFERPROC)GetGLFuncAddress("glBindBuffer"); 
   glUseProgram = (PFNGLUSEPROGRAMPROC)GetGLFuncAddress("glUseProgram"); 
   glShaderSource = (PFNGLSHADERSOURCEPROC)GetGLFuncAddress("glShaderSource"); 
   glCompileShader = (PFNGLCOMPILESHADERPROC)GetGLFuncAddress("glCompileShader"); 
   glGetShaderiv = (PFNGLGETSHADERIVPROC)GetGLFuncAddress("glGetShaderiv"); 
   glCreateShader = (PFNGLCREATESHADERPROC)GetGLFuncAddress("glCreateShader"); 
   glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)GetGLFuncAddress("glGetShaderInfoLog"); 
   glCreateProgram = (PFNGLCREATEPROGRAMPROC)GetGLFuncAddress("glCreateProgram"); 
   glAttachShader = (PFNGLATTACHSHADERPROC)GetGLFuncAddress("glAttachShader"); 
   glLinkProgram = (PFNGLLINKPROGRAMPROC)GetGLFuncAddress("glLinkProgram"); 
   glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)GetGLFuncAddress("glMapBufferRange"); 
   glMapBuffer = (PFNGLMAPBUFFERPROC)GetGLFuncAddress("glMapBuffer"); 
   glGetProgramiv = (PFNGLGETPROGRAMIVPROC)GetGLFuncAddress("glGetProgramiv"); 
   glUniform1i = (PFNGLUNIFORM1IPROC)GetGLFuncAddress("glUniform1i"); 
   glUniform1iv = (PFNGLUNIFORM1IVPROC)GetGLFuncAddress("glUniform1iv"); 
   glUniform1f = (PFNGLUNIFORM1FPROC)GetGLFuncAddress("glUniform1f"); 
   glUniform2fv = (PFNGLUNIFORM2FVPROC)GetGLFuncAddress("glUniform2fv"); 
   glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)GetGLFuncAddress("glUniformMatrix4fv");
   glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)GetGLFuncAddress("glGetUniformLocation"); 
   glDeleteShader = (PFNGLDELETESHADERPROC)GetGLFuncAddress("glDeleteShader"); 
   glDeleteProgram = (PFNGLDELETEPROGRAMPROC)GetGLFuncAddress("glDeleteProgram"); 
   glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)GetGLFuncAddress("glGenVertexArrays"); 
   glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)GetGLFuncAddress("glDrawElementsInstanced"); 
   glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)GetGLFuncAddress("glDrawArraysInstanced"); 
   glBufferData = (PFNGLBUFFERDATAPROC)GetGLFuncAddress("glBufferData"); 
   glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)GetGLFuncAddress("glBindVertexArray"); 
   glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)GetGLFuncAddress("glGetProgramInfoLog"); 
   glActiveTexture = (PFNGLACTIVETEXTUREPROC)GetGLFuncAddress("glActiveTexture");
   glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)GetGLFuncAddress("glVertexAttribPointer"); 
   glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)GetGLFuncAddress("glVertexAttribIPointer"); 
   glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)GetGLFuncAddress("glVertexAttribDivisor"); 
   glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)GetGLFuncAddress("glEnableVertexAttribArray"); 
   glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)GetGLFuncAddress("glGenerateMipmap"); 
   glGenFramebuffers =  (PFNGLGENFRAMEBUFFERSPROC)GetGLFuncAddress("glGenFramebuffers");
   glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)GetGLFuncAddress("glFramebufferTexture2D") ;
   glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)GetGLFuncAddress("glBindFramebuffer");
   glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)GetGLFuncAddress("glCheckFramebufferStatus");
   glDrawBuffers = (PFNGLDRAWBUFFERSPROC)GetGLFuncAddress("glDrawBuffers");
   glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)GetGLFuncAddress("glBindRenderbuffer");
   glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)GetGLFuncAddress("glRenderbufferStorage");
   glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)GetGLFuncAddress("glFramebufferRenderbuffer");
   glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)GetGLFuncAddress("glGenRenderbuffers");
   glUniform3f = (PFNGLUNIFORM3FPROC) GetGLFuncAddress("glUniform3f");
   glTexImage3D = (PFNGLTEXIMAGE3DPROC)GetGLFuncAddress("glTexImage3D");
   glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC)GetGLFuncAddress("glBindImageTexture");
   glMemoryBarrier = (PFNGLMEMORYBARRIERPROC)GetGLFuncAddress("glMemoryBarrier");
   glCopyImageSubData = (PFNGLCOPYIMAGESUBDATAPROC)GetGLFuncAddress("glCopyImageSubData");
   glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)GetGLFuncAddress("glBlendFuncSeparate");

   glDeleteFramebuffers =  (PFNGLDELETEFRAMEBUFFERSPROC)GetGLFuncAddress("glDeleteFramebuffers");
   glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)GetGLFuncAddress("glBlitFramebuffer");

   //and on and on and on......
}



static b32 Win32InitOpenGL(HDC* device_context, HINSTANCE Instance){
   
    PIXELFORMATDESCRIPTOR fakePFD;
    ZeroMemory(&fakePFD, sizeof(fakePFD));
    fakePFD.nSize = sizeof(fakePFD);
    fakePFD.nVersion = 1;
    fakePFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    fakePFD.iPixelType = PFD_TYPE_RGBA;
    fakePFD.cColorBits = 32;
    fakePFD.cAlphaBits = 8;
    fakePFD.cDepthBits = 24;
     
    int fakePFDID = ChoosePixelFormat(*device_context, &fakePFD);
    //NOTE(ilias): we can assign a PIXELFORMAT ONCE for every window!!
    SetPixelFormat(*device_context, fakePFDID, &fakePFD);


    //we create a temporary rendering context and set it as current
    HGLRC fakeRC = wglCreateContext(*device_context);    // Rendering Contex
    wglMakeCurrent(*device_context, fakeRC);

    {
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)GetGLFuncAddress("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)GetGLFuncAddress("wglCreateContextAttribsARB");
        wglMakeContextCurrentARB = (PFNWGLMAKECONTEXTCURRENTARBPROC)GetGLFuncAddress("wglMakeContextCurrentARB");
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)GetGLFuncAddress("wglSwapIntervalEXT");
        LoadAllOpenGLProcedures();
    }
    //now we can call wglChoosePixelFormatARB() and wglCreateContextAttribsARB()
    //we need to repeat for every function we need to get


    const int pixelAttribs[] = {
    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
    WGL_COLOR_BITS_ARB, 32,
    WGL_ALPHA_BITS_ARB, 8,
    WGL_DEPTH_BITS_ARB, 24,
    WGL_STENCIL_BITS_ARB, 8,
    WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
    WGL_SAMPLES_ARB, 4,
    0
    };
    i32 pixelFormatID;
    u32 numFormats;
    b32 status = wglChoosePixelFormatARB(*device_context, pixelAttribs, NULL, 1, &pixelFormatID, &numFormats);
    
    //DescribePixelFormat(*device_context, pixelFormatID, sizeof(PFD), &PFD); //maybe not needed
    //SetPixelFormat(*device_context, pixelFormatID, &PFD); //maybe not needed


    const int major_min = 4, minor_min = 6;     
    int  contextAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
        WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
     
    win32_opengl_context = wglCreateContextAttribsARB(*device_context, 0, contextAttribs);
    if (win32_opengl_context) {
        //NOTE(ilias): we make the new Rendering Context the current one!
        wglMakeCurrent(*device_context, NULL);
        wglDeleteContext(fakeRC);
        wglMakeCurrent(*device_context, win32_opengl_context);
        wglSwapIntervalEXT(0);
        status = 1;
    }
        return status;
}


