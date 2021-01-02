#include "renderer.h"
#include "fbo.h"
#include "tools.h"

void
renderer_init(Renderer *rend)
{
    rend->multisampling = FALSE;
    rend->multisamping_count = 4;
    rend->depthpeeling = FALSE;
    rend->depthpeel_count = 1;
    rend->renderer_settings.render_dim = (ivec2){global_platform.window_width, global_platform.window_height};
    rend->renderer_settings.lighting_disabled = FALSE;



    rend->main_fbo = fbo_init(rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0 | FBO_DEPTH);
    rend->ui_fbo = fbo_init(rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0);
    rend->shadowmap_fbo = fbo_init(rend->renderer_settings.render_dim.x * 2, rend->renderer_settings.render_dim.y * 2, FBO_DEPTH);
    rend->depthpeel_fbo = fbo_init(rend->renderer_settings.render_dim.x * 2, rend->renderer_settings.render_dim.y * 2, FBO_COLOR_0 | FBO_DEPTH);
    rend->current_fbo = &rend->main_fbo;
    

    rend->model_alloc_pos = 0;

    rend->default_material = material_default();

    shader_load(&rend->shaders[0],"../assets/shaders/mesh.vert","../assets/shaders/mesh.frag");
}

void
renderer_begin_frame(Renderer *rend)
{
  rend->renderer_settings.render_dim = (ivec2){global_platform.window_width, global_platform.window_height};

  fbo_resize(&rend->main_fbo, rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0|FBO_DEPTH);
  fbo_bind(&rend->main_fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.7,0.8,1,1);

  fbo_resize(&rend->ui_fbo, rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0|FBO_DEPTH);
  fbo_resize(&rend->shadowmap_fbo, rend->renderer_settings.render_dim.x*2, rend->renderer_settings.render_dim.y*2, FBO_COLOR_0|FBO_DEPTH);
  fbo_resize(&rend->depthpeel_fbo, rend->renderer_settings.render_dim.x*2, rend->renderer_settings.render_dim.y*2, FBO_COLOR_0|FBO_DEPTH);
  rend->current_fbo = &rend->main_fbo;
  rend->model_alloc_pos = 0;
}

void
renderer_end_frame(Renderer *rend)
{
  fbo_bind(&rend->main_fbo);
  for(i32 i = 0; i < rend->model_alloc_pos;++i)
  { 
    RendererModelData data = rend->model_instance_data[i];

    use_shader(&rend->shaders[0]);
    shader_set_int(&rend->shaders[0], "sampler", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, data.diff->id);
    shader_set_int(&rend->shaders[0], "m.specular", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, data.spec->id);
    shader_set_mat4fv(&rend->shaders[0], "model", (GLfloat*)data.model.elements);
    shader_set_mat4fv(&rend->shaders[0], "view", (GLfloat*)rend->view.elements);
    shader_set_mat4fv(&rend->shaders[0], "proj", (GLfloat*)rend->proj.elements);

    glBindVertexArray(data.model_vao);
    glDrawArrays(GL_TRIANGLES,0, data.model_vertex_count);
    glBindVertexArray(0);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0,0,rend->renderer_settings.render_dim.x,rend->renderer_settings.render_dim.y);

  fbo_copy_contents(rend->main_fbo.fbo,0);
}

/*
typedef struct RendererModelData
{
  mat4 model;
  GLuint model_vao;
  u32 model_vertex_count;
  Material *material;
  Texture *diff;
  Texture *spec;
}RendererModelData;
*/


void renderer_push_model(Renderer *rend, Model *m)
{
  RendererModelData data = (RendererModelData){0};
  data.model = mat4_translate(m->position);
  data.model_vao = m->vao;
  data.model_vertex_count = m->mesh->vertices_count;
  data.diff = &m->diff;
  data.spec = &m->spec;
  rend->model_instance_data[rend->model_alloc_pos++] = data;

}

