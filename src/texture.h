#ifndef TEXTURE_H
#define TEXTURE_H

#include "tools.h"
#include "platform.h" //just for the OpenGL function pointers
extern char info_log[512]; 

typedef enum IMAGE_FORMAT
{
    RGB = 1,
    RGBA,
    IMAGE_FORMATS_COUNT,
}IMAGE_FORMAT;

typedef struct Texture 
{
    char name[64];
    GLuint id;
    u32 width;
    u32 height;
    f32 *image_data;
    b32 has_mips;
    IMAGE_FORMAT format;
}Texture;

internal b32 texture_load(Texture* tex,char *filename)
{
    b32 result = 0;
    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    TGAInfo *image = tga_load(filename);
    if (image && image->status == TGA_OK)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->image_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        tex->has_mips = TRUE;
        tex->format = RGBA;
        result = 1;
    }else
      sprintf(error_log, "Texture: %s not found!", filename);
    //tga_destroy(image);
    tex->width = image->width;
    tex->height = image->height;
    sprintf(tex->name, filename);
    return result;
}

internal b32 texture_load_default(Texture* tex, vec4 color)
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

    vec4 *data = (vec4*)malloc(sizeof(vec4) * 512 * 512);
    for (u32 i = 0; i < 512; ++i)
    {
        for (u32 j = 0; j < 512; ++j)
        {
            data[i + j * 512] = color;
            if (i < (512/30) || j < (512/30) || i > (512/30)*29 || j > (512/30)*29)
                data[i + j * 512] = v4(0.5,0.5,0.5,1.f);

        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, 512, 512, 0, GL_RGBA, GL_FLOAT, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    result = 1;
    free(data);
    tex->width = 512;
    tex->height = 512;
    return result;
}



typedef struct TextureHandle
{
   u32 handle; // :) 
}TextureHandle;

#define MAX_TEXTURES 1024 
#define INVALID_HANDLE 0

typedef struct TextureManager
{
    Texture textures[MAX_TEXTURES];
    TextureHandle handles[MAX_TEXTURES];
    u32 next_index; //this is index in the textures array
    u32 next_handle; //this is for texture handles
    IntHashMap table; //{TextureHandle, index} pairs
}TextureManager;

internal TextureHandle tm_gen_handle(TextureManager *manager)
{
    TextureHandle handle;
    handle.handle = manager->next_handle++;
    return handle; //:)
}

internal TextureHandle
tm_load_texture(TextureManager *manager, char *filename)
{
  TextureHandle handle = tm_gen_handle(manager);
  assert(handle.handle != INVALID_HANDLE);

  hashmap_insert(&manager->table, handle.handle, manager->next_index);

  manager->textures[manager->next_index] = (Texture){0}; 
  manager->handles[manager->next_index] = handle;

  //load the texture
  texture_load(&manager->textures[manager->next_index],filename);
  sprintf(info_log, "number of textures in manager: %i", manager->next_index+1);

  //return a handle to it
  return manager->handles[manager->next_index++];
}

internal void 
tm_remove_texture(TextureManager* manager, TextureHandle handle)
{
    u32 index = hashmap_lookup(&manager->table, handle.handle);

    //if texture not found we dont need to do anything
    if (index != -1)
    {

        if (index < manager->next_index)
        {
          manager->textures[index] = manager->textures[manager->next_index-1];
          manager->handles[index] = manager->handles[manager->next_index-1];

          hashmap_remove(&manager->table,handle.handle);
          hashmap_remove(&manager->table,manager->handles[index].handle);
          hashmap_insert(&manager->table,manager->handles[index].handle, index); 
        }

        manager->next_index--;
    }
}

internal Texture* 
tm_get_texture(TextureManager *manager, TextureHandle handle)
{
    i32 index = hashmap_lookup(&manager->table, handle.handle);

    if (index != -1)
    {
        return &manager->textures[index];
    }

    return NULL;
}

TextureManager tm;

internal void
tm_init(TextureManager *manager)
{
    manager->table = hashmap_create(20);
    manager->next_index = 0;
    manager->next_handle = 1;
}

internal void
tm_bind(TextureManager *manager, TextureHandle handle, u32 texture_slot)
{
    if (handle.handle > 1024)return;
    if (handle.handle) //if (handle.handle == 0) this texture is invalid
    {
        glActiveTexture(GL_TEXTURE0 + texture_slot);
        glBindTexture(GL_TEXTURE_2D, tm_get_texture(manager, handle)->id);
    }
}



#endif
