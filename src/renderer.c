#include "renderer.h"
#include "fbo.h"
#include "skybox.h"
#include "tools.h"

local_persist char point_attr[7][64] = {
        "point_lights[x].position",
        "point_lights[x].ambient",
        "point_lights[x].diffuse",
        "point_lights[x].specular",

        "point_lights[x].constant",
        "point_lights[x].linear",
        "point_lights[x].quadratic",
};
local_persist char big_point_attr[7][64] = {
        "point_lights[xx].position",
        "point_lights[xx].ambient",
        "point_lights[xx].diffuse",
        "point_lights[xx].specular",

        "point_lights[xx].constant",
        "point_lights[xx].linear",
        "point_lights[xx].quadratic",
};

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

    rend->directional_light = (DirLight){v3(-0.2,-1,-0.3),v3(0.2,0.1,0.1),v3(0.5,0.4,0.4),v3(0.7,0.6,0.6)};
    rend->point_light_count = 0;

    char **faces= cubemap_default();
    skybox_init(&rend->skybox, faces);

    shader_load(&rend->shaders[0],"../assets/shaders/phong.vert","../assets/shaders/phong.frag");
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
  rend->point_light_count = 0;
}

void
renderer_end_frame(Renderer *rend)
{
  fbo_bind(&rend->main_fbo);
  mat4 inv_view = mat4_inv(rend->view);
  vec3 view_pos = v3(inv_view.elements[3][0],inv_view.elements[3][1],inv_view.elements[3][2]);

  skybox_render(&rend->skybox, rend->proj, rend->view);
  for(i32 i = 0; i < rend->model_alloc_pos;++i)
  { 
    RendererModelData data = rend->model_instance_data[i];

    use_shader(&rend->shaders[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, data.diff->id);
    shader_set_int(&rend->shaders[0], "material.diffuse", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, data.spec->id);
    shader_set_int(&rend->shaders[0], "material.specular", 1);
    shader_set_mat4fv(&rend->shaders[0], "model", (GLfloat*)data.model.elements);
    shader_set_mat4fv(&rend->shaders[0], "view", (GLfloat*)rend->view.elements);
    shader_set_mat4fv(&rend->shaders[0], "proj", (GLfloat*)rend->proj.elements);
    shader_set_vec3(&rend->shaders[0], "view_pos", view_pos);
    //set material properties
    shader_set_float(&rend->shaders[0], "material.shininess", data.material->shininess);
    //light properties
    shader_set_int(&rend->shaders[0], "point_light_count", rend->point_light_count);
    for (i32 i = 0; i < rend->point_light_count;++i)
    {
      if (i < 10)
      {
        point_attr[0][13] = '0'+i;
        point_attr[1][13] = '0'+i;
        point_attr[2][13] = '0'+i;
        point_attr[3][13] = '0'+i;
        point_attr[4][13] = '0'+i;
        point_attr[5][13] = '0'+i;
        point_attr[6][13] = '0'+i;
        shader_set_vec3(&rend->shaders[0],point_attr[0], rend->point_lights[i].position);
        shader_set_vec3(&rend->shaders[0],point_attr[1], rend->point_lights[i].ambient);
        shader_set_vec3(&rend->shaders[0],point_attr[2], rend->point_lights[i].diffuse);
        shader_set_vec3(&rend->shaders[0],point_attr[3], rend->point_lights[i].specular);
        shader_set_float(&rend->shaders[0],point_attr[4], rend->point_lights[i].constant);
        shader_set_float(&rend->shaders[0],point_attr[5], rend->point_lights[i].linear);
        shader_set_float(&rend->shaders[0],point_attr[6], rend->point_lights[i].quadratic);
      }
      else
      {

        point_attr[0][13] = '0'+ (i / 10);
        point_attr[0][14] = '0'+ (i % 10);
        shader_set_vec3(&rend->shaders[0],big_point_attr[0], rend->point_lights[i].position);
        shader_set_vec3(&rend->shaders[0],big_point_attr[1], rend->point_lights[i].ambient);
        shader_set_vec3(&rend->shaders[0],big_point_attr[2], rend->point_lights[i].diffuse);
        shader_set_vec3(&rend->shaders[0],big_point_attr[3], rend->point_lights[i].specular);
        shader_set_float(&rend->shaders[0],big_point_attr[4], rend->point_lights[i].constant);
        shader_set_float(&rend->shaders[0],big_point_attr[5], rend->point_lights[i].linear);
        shader_set_float(&rend->shaders[0],big_point_attr[6], rend->point_lights[i].quadratic);
      }
    }
    //directional light properties
    shader_set_vec3(&rend->shaders[0], "dirlight.direction", rend->directional_light.direction);
    shader_set_vec3(&rend->shaders[0], "dirlight.ambient", rend->directional_light.ambient);
    shader_set_vec3(&rend->shaders[0], "dirlight.diffuse", rend->directional_light.diffuse);
    shader_set_vec3(&rend->shaders[0], "dirlight.specular", rend->directional_light.specular);



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
  data.material = ALLOC(sizeof(Material));
  *data.material = material_default();
  rend->model_instance_data[rend->model_alloc_pos++] = data;

}
void renderer_push_point_light(Renderer *rend, PointLight l)
{
  rend->point_lights[rend->point_light_count++] = l;
}


