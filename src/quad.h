#ifndef QUAD_H
#define QUAD_H
#include "platform.h"
#include "shader.h"
#include "texture.h"

typedef struct Quad
{
    Shader shader;
    Texture texture;
    GLuint VAO;
}Quad;


static f32 quad_vertices[] = {
     // positions          // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f,   // top right
    -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,    // top left
     0.5f, -0.5f, 0.0f,   1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   // bottom left
};
internal void 
quad_init(Quad* q, char *tex_name)
{
    GLuint VBO;
    shader_load(&q->shader,"../assets/shaders/quad.vert", "../assets/shaders/quad.frag");
    if (!texture_load(&q->texture,tex_name))
        sprintf(error_log, "texture %s not found", tex_name); //see? this is how we stop the program because of a fatal error
    glGenVertexArrays(1, &q->VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(q->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,5 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,5 * sizeof(f32),(void*)(3 * sizeof(f32)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

extern mat4 view, proj;

internal void 
quad_render(Quad* q)
{
    mat4 mvp = mat4_mul(proj, mat4_mul(view, mat4_translate(v3(0,0,0)))); 
    use_shader(&q->shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, q->texture.id);
    shader_set_mat4fv(&q->shader, "MVP", (float*)mvp.elements);
    glBindVertexArray(q->VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
#endif
