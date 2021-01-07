#ifndef FBO_H
#define FBO_H
#include "tools.h"
#include "platform.h"

//NOTE: used in initialization to
//figure out which components we want
#define FBO_COLOR_0  0x01
#define FBO_COLOR_1  0x02
#define FBO_COLOR_2  0x04
#define FBO_COLOR_3  0x08
#define FBO_DEPTH    0x10

typedef struct OpenGLFBO
{
   i32 flags;
   GLuint fbo;
   GLuint color_attachments[4];
   GLuint depth_attachment;

   union
   {
        struct
        {
            u32 w,h;
        };
        struct 
        {
            u32 width, height;
        };
   };
}OpenGLFBO;

static OpenGLFBO 
fbo_init(u32 width, u32 height, i32 flags)
{
    OpenGLFBO fbo = {0};
    fbo.flags = flags;
    fbo.width = width + 1;
    fbo.height = height + 1;
    glGenFramebuffers(1, &fbo.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
    {
        GLuint colors[4] = {0};
        u32 color_count = 0;
        
        if(flags & FBO_COLOR_0)
        {
            glGenTextures(1, fbo.color_attachments+0);
            glBindTexture(GL_TEXTURE_2D, fbo.color_attachments[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.color_attachments[0], 0);
            colors[color_count++] = GL_COLOR_ATTACHMENT0;
        }
        
        if(flags & FBO_COLOR_1)
        {
            glGenTextures(1, fbo.color_attachments+1);
            glBindTexture(GL_TEXTURE_2D, fbo.color_attachments[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fbo.color_attachments[1], 0);
            colors[color_count++] = GL_COLOR_ATTACHMENT1;
        }
        
        if(flags & FBO_COLOR_2)
        {
            glGenTextures(1, fbo.color_attachments+2);
            glBindTexture(GL_TEXTURE_2D, fbo.color_attachments[2]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, fbo.color_attachments[2], 0);
            colors[color_count++] = GL_COLOR_ATTACHMENT2;
        }
        
        if(flags & FBO_COLOR_3)
        {
            glGenTextures(1, fbo.color_attachments+3);
            glBindTexture(GL_TEXTURE_2D, fbo.color_attachments[3]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, fbo.color_attachments[3], 0);
            colors[color_count++] = GL_COLOR_ATTACHMENT3;
        }
        
        if(flags & FBO_DEPTH)
        {
            glGenTextures(1, &fbo.depth_attachment);
            glBindTexture(GL_TEXTURE_2D, fbo.depth_attachment);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo.depth_attachment, 0);
        }
        
        glDrawBuffers(color_count, colors);
        
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbo;
};

static void
fbo_cleanup(OpenGLFBO *fbo)
{
    glDeleteFramebuffers(1, &fbo->fbo);
    for(u32 i = 0; i < array_count(fbo->color_attachments); ++i)
    {
        if(fbo->color_attachments[i])
        {
            glDeleteTextures(1, &fbo->color_attachments[i]);
        }
    }
    if(fbo->depth_attachment)
    {
        glDeleteTextures(1, &fbo->depth_attachment);
    }
    fbo->fbo = 0;
}

//it FORCES the fbo to the size
static void
fbo_resize(OpenGLFBO *fbo, u32 w, u32 h, i32 flags)
{
    u32 adjusted_width = w + 1;
    u32 adjusted_height = h + 1;
    fbo_cleanup(fbo);
    *fbo = fbo_init(w, h, flags);
}

static void
fbo_bind(OpenGLFBO *fbo)
{
    if(fbo)
    {
        glViewport(0, 0, (GLsizei)fbo->w, (GLsizei)fbo->h);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glViewport(0, 0, (GLsizei)global_platform.window_width, (GLsizei)global_platform.window_height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }
    //renderer->active_fbo = fbo;
}

static void
fbo_clear(OpenGLFBO *fbo)
{
    GLuint last_fbo_bound;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &last_fbo_bound);

    bind_fbo(fbo);
    //clear to white
    glClearColor(0.f, 0.f, 0.f, 0.f);
    if(fbo->depth_attachment)
    {
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    bind_fbo(last_fbo_bound);
}

//please make sure the framebuffers are of the same size
//I did it this way so you can copy to framebuffer 0 
static void 
fbo_copy_contents(GLuint src_fbo,GLuint dest_fbo)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest_fbo);
    
glBlitFramebuffer(0, 0, global_platform.window_width, global_platform.window_height, 0, 0, 
        global_platform.window_width, global_platform.window_height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

static void 
fbo_blend_contents(GLuint src_fbo,GLuint dest_fbo)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest_fbo);
    
glBlitFramebuffer(0, 0, global_platform.window_width, global_platform.window_height, 0, 0, 
        global_platform.window_width, global_platform.window_height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

#endif
