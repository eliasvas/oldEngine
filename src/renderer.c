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
local_persist f32 screen_verts[] = { 
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
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
    rend->postproc_fbo = fbo_init(rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0 | FBO_DEPTH);
    rend->ui_fbo = fbo_init(rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0);
    rend->shadowmap_fbo = fbo_init(1024 * 2, 1024 * 2, FBO_DEPTH);
    rend->depthpeel_fbo = fbo_init(rend->renderer_settings.render_dim.x * 2, rend->renderer_settings.render_dim.y * 2, FBO_COLOR_0 | FBO_DEPTH);
    rend->current_fbo = &rend->main_fbo;
    

    rend->model_alloc_pos = 0;

    rend->default_material = material_default();

    rend->directional_light = (DirLight){v3(-0.2,-1,-0.3),v3(0.8,0.7,0.7),v3(0.8,0.7,0.7),v3(0.9f,0.9f,0.9f)};
    rend->point_light_count = 0;

    char **faces= cubemap_default();
    skybox_init(&rend->skybox, faces);

    //initialize postproc VAO
    {
        GLuint quad_vbo;
        glGenVertexArrays(1, &rend->postproc_vao);
        glGenBuffers(1, &quad_vbo);
        glBindVertexArray(rend->postproc_vao);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(screen_verts), &screen_verts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(GLfloat)));
        glBindVertexArray(0); 
    }

    shader_load(&rend->shaders[0],"../assets/shaders/phong.vert","../assets/shaders/phong.frag");
    shader_load(&rend->shaders[1],"../assets/shaders/skybox_reflect.vert","../assets/shaders/skybox_reflect.frag");
    shader_load(&rend->shaders[2],"../assets/shaders/postproc.vert","../assets/shaders/postproc.frag");
    shader_load(&rend->shaders[3],"../assets/shaders/shadowmap.vert","../assets/shaders/shadowmap.frag");
}

void
renderer_begin_frame(Renderer *rend)
{
  rend->renderer_settings.render_dim = (ivec2){global_platform.window_width, global_platform.window_height};

  fbo_resize(&rend->postproc_fbo, rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0|FBO_DEPTH);
  fbo_bind(&rend->postproc_fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0,0,0,0);

  fbo_bind(&rend->shadowmap_fbo);
  glClear(GL_DEPTH_BUFFER_BIT);
  //glClearColor(1,1,1,1);



  fbo_resize(&rend->main_fbo, rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0|FBO_DEPTH);
  fbo_bind(&rend->main_fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.7,0.8,1,1);

  fbo_resize(&rend->ui_fbo, rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0|FBO_DEPTH);
  fbo_bind(&rend->main_fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0,0,0,0);
  fbo_resize(&rend->depthpeel_fbo, rend->renderer_settings.render_dim.x*2, rend->renderer_settings.render_dim.y*2, FBO_COLOR_0|FBO_DEPTH);
  rend->current_fbo = &rend->main_fbo;
  rend->model_alloc_pos = 0;
  rend->point_light_count = 0;
}


//here we put the draw calls for everything that needs shadowmapping!
internal void
renderer_render_scene3D(Renderer *rend,Shader *shader)
{
  mat4 inv_view = mat4_inv(rend->view);
  vec3 view_pos = v3(inv_view.elements[3][0],inv_view.elements[3][1],inv_view.elements[3][2]);
  for(i32 i = 0; i < rend->model_alloc_pos;++i)
  { 
    RendererModelData data = rend->model_instance_data[i];
    mat4 ortho_proj = orthographic_proj(-20.f, 20.f, -20.f, 20.f, 0.01f, 50.f);
    //mat4 light_space_matrix = mat4_mul(ortho_proj, mat4_mul(
     //     mat4_translate(v3(rend->view.elements[3][0],rend->view.elements[3][1] + 10.f,rend->view.elements[3][2])), mat4_rotate(-90.f, v3(1.f,0.f,0.f))));

    mat4 light_space_matrix = mat4_mul(ortho_proj,look_at(v3(0,20,0), v3(10,0,0), v3(0,1,0)));



    use_shader(&shader[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, data.diff->id);
    shader_set_int(&shader[0], "material.diffuse", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, data.spec->id);
    shader_set_int(&shader[0], "material.specular", 1);

    //in case we need the skybox's texture for the rendering
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, rend->skybox.tex_id);
    shader_set_int(&rend->skybox.shader, "skybox", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, rend->shadowmap_fbo.depth_attachment);
    shader_set_int(&shader[0], "shadow_map", 4);


    shader_set_mat4fv(&shader[0], "model", (GLfloat*)data.model.elements);
    shader_set_mat4fv(&shader[0], "view", (GLfloat*)rend->view.elements);
    shader_set_mat4fv(&shader[0], "proj", (GLfloat*)rend->proj.elements);
    shader_set_mat4fv(&shader[0], "light_space_matrix", (GLfloat*)light_space_matrix.elements);
    shader_set_vec3(&shader[0], "view_pos", view_pos);
    //set material properties
    shader_set_float(&shader[0], "material.shininess", data.material->shininess);
    //light properties
    shader_set_int(&shader[0], "point_light_count", rend->point_light_count);
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
        shader_set_vec3(&shader[0],point_attr[0], rend->point_lights[i].position);
        shader_set_vec3(&shader[0],point_attr[1], rend->point_lights[i].ambient);
        shader_set_vec3(&shader[0],point_attr[2], rend->point_lights[i].diffuse);
        shader_set_vec3(&shader[0],point_attr[3], rend->point_lights[i].specular);
        shader_set_float(&shader[0],point_attr[4], rend->point_lights[i].constant);
        shader_set_float(&shader[0],point_attr[5], rend->point_lights[i].linear);
        shader_set_float(&shader[0],point_attr[6], rend->point_lights[i].quadratic);
      }
      else
      {

        point_attr[0][13] = '0'+ (i / 10);
        point_attr[0][14] = '0'+ (i % 10);
        shader_set_vec3(&shader[0],big_point_attr[0], rend->point_lights[i].position);
        shader_set_vec3(&shader[0],big_point_attr[1], rend->point_lights[i].ambient);
        shader_set_vec3(&shader[0],big_point_attr[2], rend->point_lights[i].diffuse);
        shader_set_vec3(&shader[0],big_point_attr[3], rend->point_lights[i].specular);
        shader_set_float(&shader[0],big_point_attr[4], rend->point_lights[i].constant);
        shader_set_float(&shader[0],big_point_attr[5], rend->point_lights[i].linear);
        shader_set_float(&shader[0],big_point_attr[6], rend->point_lights[i].quadratic);
      }
    }
    //directional light properties
    shader_set_vec3(&shader[0], "dirlight.direction", rend->directional_light.direction);
    shader_set_vec3(&shader[0], "dirlight.ambient", rend->directional_light.ambient);
    shader_set_vec3(&shader[0], "dirlight.diffuse", rend->directional_light.diffuse);
    shader_set_vec3(&shader[0], "dirlight.specular", rend->directional_light.specular);



    glBindVertexArray(data.model_vao);
    glDrawArrays(GL_TRIANGLES,0, data.model_vertex_count);
    glBindVertexArray(0);
  }

}

void
renderer_end_frame(Renderer *rend)
{
  mat4 inv_view = mat4_inv(rend->view);
  vec3 view_pos = v3(inv_view.elements[3][0],inv_view.elements[3][1],inv_view.elements[3][2]);

  //first we render the scene to the depth map
  fbo_bind(&rend->shadowmap_fbo);
  renderer_render_scene3D(rend,&rend->shaders[3]);
  //then we render to the main fbo
  fbo_bind(&rend->main_fbo);
  renderer_render_scene3D(rend,&rend->shaders[0]);

  //at the end we render the skybox
  skybox_render(&rend->skybox, rend->proj, rend->view);

  glBindFramebuffer(GL_FRAMEBUFFER, rend->postproc_fbo.fbo);
  glBindTexture(GL_TEXTURE_2D, rend->main_fbo.color_attachments[0]);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y);

    glEnable(GL_DEPTH_TEST);
    use_shader(&rend->shaders[2]);
    glBindVertexArray(rend->postproc_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rend->main_fbo.color_attachments[0]);
    shader_set_int(&rend->shaders[2],"screenTexture",0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0,0,rend->renderer_settings.render_dim.x,rend->renderer_settings.render_dim.y);

  //fbo_copy_contents(rend->main_fbo.fbo,0);
  fbo_copy_contents(rend->postproc_fbo.fbo,0);
  //fbo_copy_contents(rend->ui_fbo.fbo,0);
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


