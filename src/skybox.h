#ifndef SKYBOX_H
#define SKYBOX_H


#include "tools.h"

internal char **cubemap_default()
{
  char *res[6] = {
        "../assets/vrb/vrbft.tga",
                "../assets/vrb/vrbbk.tga",
        "../assets/vrb/vrbdn.tga", //up
        "../assets/vrb/vrbup.tga", //dn
"../assets/vrb/vrbrt.tga",
        "../assets/vrb/vrblf.tga",

  };
  return res;
}

internal u32 
cubemap_load(char **faces)
{
    u32 textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    i32 width, height, nrChannels;
    for (u32 i = 0; i < 6; i++)
    {
        TGAInfo *image = tga_load(faces[i]);
        if (image && image->status == TGA_OK)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->image_data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            sprintf(error_log, "cubemap texture at: %s failed to load!\n", faces[i]);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


typedef struct Skybox {
	
	u32 vao;
	u32 vbo;
    u32 tex_id;
    char **faces;
    Shader shader;
}Skybox;

internal void 
skybox_load(Skybox *skybox,const char**faces) {
    skybox->faces = faces;
    skybox->tex_id = cubemap_load(faces);
}

internal void
skybox_init(Skybox* skybox, char**faces) {
    shader_load(&skybox->shader,"../assets/shaders/skybox.vert", "../assets/shaders/skybox.frag");
    skybox_load(skybox,faces);
    f32 skybox_vertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    glGenVertexArrays(1, &skybox->vao);
    glGenBuffers(1, &skybox->vbo);
    glBindVertexArray(skybox->vao);
    glBindBuffer(GL_ARRAY_BUFFER, skybox->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);	
    glBindVertexArray(0);
}

internal void 
skybox_render(Skybox* skybox, mat4 proj, mat4 view) {
    use_shader(&skybox->shader);
    //glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(skybox->vao);
    shader_set_mat4fv(&skybox->shader, "uniform_projection_matrix", (float*)proj.elements);
    shader_set_mat4fv(&skybox->shader, "uniform_view_matrix", (float*)view.elements);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->tex_id);
    shader_set_int(&skybox->shader, "skybox", 0);

    glDrawArrays(GL_TRIANGLES, 0,36);
    glDepthFunc(GL_LESS);
    //glDepthMask(GL_TRUE);
}

#endif
