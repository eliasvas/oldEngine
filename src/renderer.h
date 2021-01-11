#ifndef RENDERER_H
#define RENDERER_H
#include "tools.h"
#include "shader.h"
#include "texture.h"
#include "skybox.h"
#include "model.h"
#include "fbo.h"
#include "animation.h"

typedef struct RendererSettings
{
  ivec2 render_dim;
  b32 lighting_disabled;
  b32 no_mips;
  //multisampling settings
  //depth peeling settings
}RendererSettings;

typedef struct RendererModelData
{
  mat4 model;
  GLuint model_vao;
  u32 model_vertex_count;
  Material *material;
  Texture *diff;
  Texture *spec;
}RendererModelData;

typedef struct RendererAnimatedModelData
{
    GLuint vao;
    Texture diff;
    Texture spec;
    
    u32 joint_count;
    Joint *joints;
    u32 vertices_count;
    mat4 model;
}RendererAnimatedModelData;


#define RENDERER_MAX_SHADERS 256
#define RENDERER_MAX_POINT_LIGHTS 256
#define RENDERER_BYTES_PER_MODEL sizeof(RendererModelData)
#define RENDERER_MAX_MODELS 256
#define RENDERER_MAX_ANIMATED_MODELS 64

typedef struct Renderer
{

  b32 multisampling;
  u32 multisamping_count;
  b32 depthpeeling;
  u32 depthpeel_count;
  RendererSettings renderer_settings;

  OpenGLFBO *current_fbo;
  OpenGLFBO postproc_fbo;
  OpenGLFBO main_fbo;
  OpenGLFBO ui_fbo;
  OpenGLFBO shadowmap_fbo;
  OpenGLFBO depthpeel_fbo;
  

  GLuint postproc_vao;
  GLuint model_vao;
  RendererModelData model_instance_data[RENDERER_MAX_MODELS];
  u32 model_alloc_pos;


  RendererAnimatedModelData animated_model_instance_data[RENDERER_MAX_ANIMATED_MODELS];
  u32 animated_model_alloc_pos;

  Shader shaders[RENDERER_MAX_SHADERS];
  u32 shaders_count;
  PointLight point_lights[RENDERER_MAX_POINT_LIGHTS];
  u32 point_light_count;
  DirLight directional_light;
  Material default_material;
  Skybox skybox;
  mat4 view;
  mat4 proj;
}Renderer;

void
renderer_init(Renderer *rend);

void
renderer_begin_frame(Renderer *rend);

void
renderer_end_frame(Renderer *rend);
#endif
