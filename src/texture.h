#ifndef TEXTURE_H
#define TEXTURE_H

#include "tools.h"



typedef struct Texture 
{
    GLuint id;
    u32 width;
    u32 height;
}Texture;

internal b32 texture_load(Texture* tex,const char *filename)
{
    b32 result = 0;
    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    TGAInfo *image = tga_load(filename);
    if (image && image->status == TGA_OK)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->image_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        result = 1;
    }else
      sprintf(error_log, "Texture: %s not found!", filename);
    tga_destroy(image);
    tex->width = image->width;
    tex->height = image->height;
    return result;
}

#endif
