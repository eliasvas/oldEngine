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
    MeshInfo *meshes;
    u32 mesh_count;
    Shader s;
    mat4 model;
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
    m->meshes = ALLOC(sizeof(MeshInfo));
    m->mesh_count = 1;
    m->meshes[0].vertices_count = 36;
    m->model = mat4_translate(v3(0,0,0));
    glGenVertexArrays(1, &m->meshes[0].vao);
    glBindVertexArray(m->meshes[0].vao); 
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m->meshes[0].vertices_count, &cube_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *) (sizeof(float) * 3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *) (sizeof(float) * 6));
    glBindVertexArray(0);


    shader_load(&m->s,"../assets/shaders/mesh.vert","../assets/shaders/mesh.frag");
    texture_load(&(m->meshes[0].material.diff),"../assets/texture.tga");
    texture_load(&(m->meshes[0].material.spec),"../assets/texture_spec.tga");
      
}
internal Material 
material_default(void)
{
  Material material = (Material){0};
  material.ambient = v3(0.3f,0.3f,0.3f);
  material.diffuse = v3(0.5f,0.5f,0.5f);
  material.specular = v3(0.1f,0.1f,0.1f);
  material.shininess = 1.f;
}


internal Model 
model_info_init(char *mtl_filepath)
{
  Model model_info = {0};
  shader_load(&model_info.s,"../assets/shaders/mesh.vert","../assets/shaders/mesh.frag");
  //1. read all different materials
  MeshMaterial *materials;  
  u32 materials_count = mtl_count(mtl_filepath);
  materials = ALLOC(sizeof(MeshMaterial) * materials_count);
  mtl_read(mtl_filepath, materials);
  //2. read all different meshes AND their materials and initialize the MeshInfo **mesh
  char objpath[32];
  u32 filepath_size = str_size(mtl_filepath);
  memcpy(objpath, mtl_filepath, str_size(mtl_filepath));
  objpath[filepath_size] = '\0';
  objpath[filepath_size-3] = 'o';
  objpath[filepath_size-2] = 'b';
  objpath[filepath_size-1] = 'j';
  model_info.mesh_count = obj_count_meshes(objpath);
  model_info.meshes = obj_read(objpath, materials);
  model_info.model = m4d(1.f);
  //3. read the vertices of all the different meshes

  //4. generate vertex buffers for everything

  return model_info;
}

extern mat4 view, proj;
internal void
model_render(Model *m)
{
   use_shader(&m->s);
    
    shader_set_mat4fv(&m->s, "view", (GLfloat*)view.elements);
    shader_set_mat4fv(&m->s, "proj", (GLfloat*)proj.elements);

    for (u32 i = 0; i < m->mesh_count; ++i)
    {
   
      shader_set_int(&m->s, "sampler", 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m->meshes[i].material.diff.id);
      shader_set_int(&m->s, "m.specular", 1);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, m->meshes[i].material.spec.id);

      glBindVertexArray(m->meshes[i].vao);
      //glDrawArrays(GL_TRIANGLES,0, m->meshes[i].vertices_count);
      glDrawArrays(GL_TRIANGLES,0, 10000);
      shader_set_mat4fv(&m->s, "model", (GLfloat*)m->model.elements);
    }
    glBindVertexArray(0);

}


#endif
