#ifndef MODEL_H
#define MODEL_H

#include "objloader.h"
#include "tools.h"
#include "platform.h"
#include "shader.h"
#include "texture.h"

/*
NOTE(ilias): OpenGL function pointers needed if
model.h is used in another translation unit
*/
#include "platform.h"

typedef struct Model
{
    GLuint vao;
    MeshInfo *mesh;
    Texture spec;
    Texture diff;
    Shader s;
    vec3 position;
}Model;


typedef struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    f32 shininess;
}Material;
typedef struct Light {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}Light;
typedef struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}DirLight;  
typedef struct PointLight {    
    vec3 position;
    
    f32 constant;
    f32 linear;
    f32 quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    f32 shininess;
}PointLight;  

//NOTE(ilias): these are for debug, in case the renderer
//is not available, to render models, push to renderer!

static void 
model_init(Model* m, MeshInfo *mesh)
{
    {
        m->mesh = mesh;
        //m->position = v3(random01()*3,0,0); 
    }
    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao); 
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh->vertices_count, &mesh->vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *) (sizeof(float) * 3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *) (sizeof(float) * 6));
    glBindVertexArray(0);


    shader_load(&m->s,"../assets/shaders/mesh.vert","../assets/shaders/mesh.frag");
    texture_load(&(m->diff),"../assets/arena/main3.tga");
    texture_load(&(m->spec),"../assets/arena/main3.tga");
      
}
//local_persist f32 cube_data[] = { -1.0f,-1.0f,-1.0f, 1.0f, 1.0f,-1.0f, -1.0f,-1.0f,-1.0f, -1.0f, 1.0f,-1.0f, 1.0f,-1.0f, 1.0f, -1.0f,-1.0f,-1.0f, 1.0f,-1.0f,-1.0f, 1.0f, 1.0f,-1.0f, 1.0f,-1.0f,-1.0f, -1.0f,-1.0f,-1.0f, -1.0f,-1.0f,-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,-1.0f, 1.0f,-1.0f, 1.0f, -1.0f,-1.0f, 1.0f, -1.0f,-1.0f,-1.0f, -1.0f, 1.0f, 1.0f, -1.0f,-1.0f, 1.0f, 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,-1.0f,-1.0f, 1.0f, 1.0f,-1.0f, 1.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,-1.0f, -1.0f, 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f,-1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,-1.0f, 1.0f };
local_persist f32 cube_data[] = {
        // positions          // normals           // texture coords
        -1.f, -1.f, -1.f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         1.f, -1.f, -1.f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         1.f,  1.f, -1.f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         1.f,  1.f, -1.f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -1.f,  1.f, -1.f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -1.f, -1.f, -1.f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -1.f, -1.f,  1.f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         1.f, -1.f,  1.f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         1.f,  1.f,  1.f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         1.f,  1.f,  1.f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -1.f,  1.f,  1.f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -1.f, -1.f,  1.f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -1.f,  1.f,  1.f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -1.f,  1.f, -1.f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -1.f, -1.f, -1.f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -1.f, -1.f, -1.f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -1.f, -1.f,  1.f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -1.f,  1.f,  1.f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         1.f,  1.f,  1.f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         1.f,  1.f, -1.f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         1.f, -1.f, -1.f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         1.f, -1.f, -1.f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         1.f, -1.f,  1.f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         1.f,  1.f,  1.f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -1.f, -1.f, -1.f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         1.f, -1.f, -1.f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         1.f, -1.f,  1.f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         1.f, -1.f,  1.f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -1.f, -1.f,  1.f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -1.f, -1.f, -1.f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -1.f,  1.f, -1.f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         1.f,  1.f, -1.f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         1.f,  1.f,  1.f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         1.f,  1.f,  1.f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -1.f,  1.f,  1.f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -1.f,  1.f, -1.f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
};
static void 
model_init_cube(Model* m)
{
    m->mesh = ALLOC(sizeof(MeshInfo));
    m->mesh->vertices_count = 36;
    m->position = v3(0,0,0);
    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao); 
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m->mesh->vertices_count, &cube_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *) (sizeof(float) * 3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *) (sizeof(float) * 6));
    glBindVertexArray(0);


    shader_load(&m->s,"../assets/shaders/mesh.vert","../assets/shaders/mesh.frag");
    texture_load(&(m->diff),"../assets/texture.tga");
    texture_load(&(m->spec),"../assets/texture_spec.tga");
      
}

static void 
model_render(Model* m,mat4 *proj,mat4 *view)
{
    use_shader(&m->s);
    
    shader_set_int(&m->s, "sampler", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m->diff.id);
    shader_set_int(&m->s, "m.specular", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m->spec.id);
    //mat4 mvp = mul_mat4(mv,translate_mat4(m->position));
    mat4 model =mat4_translate(m->position);
    shader_set_mat4fv(&m->s, "model", (GLfloat*)model.elements);
    shader_set_mat4fv(&m->s, "view", (GLfloat*)view->elements);
    shader_set_mat4fv(&m->s, "proj", (GLfloat*)proj->elements);

    glBindVertexArray(m->vao);
    glDrawArrays(GL_TRIANGLES,0, m->mesh->vertices_count);
    glBindVertexArray(0);
}


/*
static void 
render_model_textured(Model* m, mat4* projection, mat4* view, vec3 light_pos, vec3 camera_pos)
{
    use_shader(&m->s);
    
    setInt(&m->s, "m.diffuse", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m->diff->id);
    setInt(&m->s, "m.specular", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m->spec->id);

    Light light = {light_pos,{0.2f, 0.2f, 0.2f},{0.7f, 0.7f, 0.7f},{1.0f, 1.0f, 1.0f}};
    mat4 model = translate_mat4(m->position);
    model.elements[0][0] = m->scale.x;
    model.elements[1][1] = m->scale.y;
    model.elements[2][2] = m->scale.z;
    setMat4fv(&m->s, "proj", (GLfloat*)projection->elements);
    setMat4fv(&m->s, "view", (GLfloat*)view->elements);
    setMat4fv(&m->s, "model", (GLfloat*)model.elements);
    {
        setVec3(&m->s,"m.ambient", {0.2f, 0.2f, 0.2f});
        setVec3(&m->s,"m.diffuse", {0.4f, 0.2f, 0.1f});
        setVec3(&m->s,"m.specular", {0.1f, 0.1f, 0.1f});
        setFloat(&m->s, "m.shininess", 3.f);
    }
    {
        setVec3(&m->s,"light.ambient", light.ambient);
        setVec3(&m->s,"light.direction", {0,1,0});
        setVec3(&m->s,"light.diffuse", light.diffuse);
        setVec3(&m->s,"light.specular", light.specular);
        glUniform3f(glGetUniformLocation(m->s.ID, "light.position"), light_pos.x, light_pos.y, light_pos.z); 
    }

    glUniform3f(glGetUniformLocation(m->s.ID, "view_pos"), camera_pos.x, camera_pos.y, camera_pos.z); 

    glBindVertexArray(m->vao);
    //int mode = (int)(global_platform.current_time) % 3;
    //if (mode == 2)mode = GL_TRIANGLES;
    //glDrawArrays( mode,0, m->vertices.size());
    glDrawArrays(GL_TRIANGLES,0, m->vertices.size());
    glBindVertexArray(0);
}
*/
internal Material 
material_default(void)
{
  Material material = (Material){0};
  material.ambient = v3(0.3f,0.3f,0.3f);
  material.diffuse = v3(0.5f,0.5f,0.5f);
  material.specular = v3(0.1f,0.1f,0.1f);
  material.shininess = 1.f;
}


#endif
