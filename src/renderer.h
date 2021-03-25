#ifndef RENDERER_H
#define RENDERER_H
#include "tools.h"
#include "shader.h"
#include "texture.h"
#include "skybox.h"
#include "model.h"
#include "camera.h"
#include "fbo.h"
#include "animation.h"

typedef struct RendererSettings
{
  ivec2 render_dim;
  b32 lighting_disabled;
  b32 no_mips;
  b32 motion_blur;
  b32 z_prepass;
  b32 light_cull;
  //multisampling settings
  //depth peeling settings
}RendererSettings;

typedef struct RendererPointData
{
    vec3 pos;
    vec4 color;
}RendererPointData;
typedef struct RendererModelData
{
  mat4 model;
  GLuint model_vao;
  u32 model_vertex_count;
  Material material;
  Texture *diff;
  Texture *spec;
}RendererModelData;

typedef struct RendererFilledRect
{
    vec3 offset;
    vec2 dim; //width and height of rect
    vec4 color;
}RendererFilledRect;

typedef struct RendererLine
{
    vec3 start;
    vec3 end;
    vec4 color;
}RendererLine;


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

typedef struct RendererChar
{
    vec3 offset;
    vec2 dim;
    vec2 uv; //where in the bitmap font the character is 
}RendererChar;

typedef struct VisibleIndex
{
    u32 index;
}VisibleIndex;


#define RENDERER_MAX_SHADERS 256
#define RENDERER_MAX_POINT_LIGHTS 1024 
#define RENDERER_BYTES_PER_MODEL sizeof(RendererModelData)
#define RENDERER_MAX_MODELS 256
#define RENDERER_MAX_RECTS 256
#define RENDERER_MAX_ANIMATED_MODELS 64
#define RENDERER_MAX_TEXT 512 
#define RENDERER_MAX_POINTS 1024

typedef struct Renderer
{

  b32 multisampling;
  u32 multisampling_count;
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

  GLuint filled_rect_vao;
  GLuint filled_rect_instance_vbo;
  RendererFilledRect filled_rect_instance_data[RENDERER_MAX_RECTS];
  u32 filled_rect_alloc_pos;
  
  GLuint point_vao;
  GLuint point_vbo;
  RendererPointData point_instance_data[RENDERER_MAX_POINTS];
  u32 point_alloc_pos;

  GLuint line_vao;
  GLuint line_instance_vbo;
  RendererLine line_instance_data[RENDERER_MAX_RECTS];
  u32 line_alloc_pos;

  GLuint text_vao;
  GLuint text_instance_vbo;
  RendererChar text_instance_data[RENDERER_MAX_TEXT];
  u32 text_alloc_pos;



  RendererAnimatedModelData animated_model_instance_data[RENDERER_MAX_ANIMATED_MODELS];
  u32 animated_model_alloc_pos;

  Shader shaders[RENDERER_MAX_SHADERS];
  Camera cam;
  u32 shaders_count;
  PointLight point_lights[RENDERER_MAX_POINT_LIGHTS];
  u32 point_light_count;
  DirLight directional_light;
  Material default_material;
  Skybox skybox;
  Texture white_texture;
  Texture bmf;
  mat4 view;
  mat4 proj;

  //forward+ stuff
  GLuint light_buffer;
  GLuint visible_light_indices_buffer;
  GLuint debug_texture;
}Renderer;

void
renderer_init(Renderer *rend);

void
renderer_begin_frame(Renderer *rend);

void
renderer_end_frame(Renderer *rend);
#endif
