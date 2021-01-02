#ifndef RENDERER_H
#define RENDERER_H
#include "tools.h"
#include "shader.h"
#include "texture.h"
#include "model.h"
#include "fbo.h"

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

#define RENDERER_MAX_SHADERS 256
#define RENDERER_MAX_POINT_LIGHTS 256
#define RENDERER_BYTES_PER_MODEL sizeof(RendererModelData)
#define RENDERER_MAX_MODELS 256

typedef struct Renderer
{

  b32 multisampling;
  u32 multisamping_count;
  b32 depthpeeling;
  u32 depthpeel_count;
  RendererSettings renderer_settings;

  OpenGLFBO *current_fbo;
  OpenGLFBO main_fbo;
  OpenGLFBO ui_fbo;
  OpenGLFBO shadowmap_fbo;
  OpenGLFBO depthpeel_fbo;
  

  GLuint model_vao;
  RendererModelData model_instance_data[RENDERER_MAX_MODELS];
  u32 model_alloc_pos;

  Shader shaders[RENDERER_MAX_SHADERS];
  u32 shaders_count;
  PointLight point_lights[RENDERER_MAX_POINT_LIGHTS];
  u32 point_light_count;
  DirLight directional_light;
  Material default_material;
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
