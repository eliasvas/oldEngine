#include "renderer.h"
#include "fbo.h"
#include "skybox.h"
#include "tools.h"

extern char info_log[512]; 
local_persist char point_attr[4][64] = {
        "point_lights[x].position",
        "point_lights[x].ambient",
        "point_lights[x].diffuse",
        "point_lights[x].specular",
};
local_persist char big_point_attr[4][64] = {
        "point_lights[xx].position",
        "point_lights[xx].ambient",
        "point_lights[xx].diffuse",
        "point_lights[xx].specular",
};
local_persist char kernel_attr[64] = { "kernel[x]" };
local_persist char big_kernel_attr[64] = { "kernel[xx]" };

local_persist f32 screen_verts[] = { 
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
};

local_persist f32 filled_quad_verts[] = { 0.f,0.f, 0.f,1.f, 1.f,0.f, 1.f,1.f };

void
renderer_init(Renderer *rend)
{
    camera_init(&rend->cam);
    rend->multisampling = FALSE;
    rend->multisampling_count = 2;
    rend->depthpeeling = FALSE;
    rend->depthpeel_count = 1;
    rend->renderer_settings.render_dim = (ivec2){global_platform.window_width, global_platform.window_height};
    rend->renderer_settings.lighting_disabled = FALSE;
    rend->renderer_settings.light_cull = TRUE;
    rend->renderer_settings.z_prepass = TRUE;
    rend->renderer_settings.debug_mode = FALSE;
    rend->renderer_settings.cascaded_render = TRUE;
    rend->renderer_settings.sdf_fonts = TRUE;
    rend->renderer_settings.ssao_on = TRUE;

    //initializing the test sphere
    model_init_sphere(&rend->test_sphere, 0.1, 8, 8);

    /* Query max color attachments
    u32 max_color_attachments;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_color_attachments);
    sprintf(info_log, "max_color_attachments = %i", max_color_attachments);
    */

    rend->main_fbo = fbo_init(rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0 | FBO_COLOR_1| FBO_COLOR_2 | FBO_DEPTH);
    rend->ssao_fbo = fbo_init(rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0 |FBO_COLOR_1| FBO_DEPTH);
    rend->postproc_fbo = fbo_init(rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0 |FBO_COLOR_1| FBO_DEPTH);
    rend->ui_fbo = fbo_init(rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0);
    rend->shadowmap_fbo[0] = fbo_init(1024*2, 1024*2, FBO_DEPTH);
    rend->shadowmap_fbo[1] = fbo_init(1024, 1024, FBO_DEPTH);
    rend->shadowmap_fbo[2] = fbo_init(512, 512, FBO_DEPTH);
    //rend->depthpeel_fbo = fbo_init(rend->renderer_settings.render_dim.x * 2, rend->renderer_settings.render_dim.y * 2, FBO_COLOR_0 | FBO_DEPTH);
    rend->current_fbo = &rend->main_fbo;
    

    rend->model_alloc_pos = 0;

    rend->default_material = material_default();

    rend->directional_light = (DirLight){v3(-0.3,-0.7,-0.3),v3(0.2,0.2,0.1),v3(0.8,0.8,0.8),v3(0.8f,0.8f,0.8f)};
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

    }

    //initialize filled rect vao
    {
        GLuint vbo;
        glGenVertexArrays(1, &rend->filled_rect_vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &rend->filled_rect_instance_vbo);
        glBindVertexArray(rend->filled_rect_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(filled_quad_verts), &filled_quad_verts[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        //this buffer should be update with the contents of filled rect instance data before rendering
        glBindBuffer(GL_ARRAY_BUFFER, rend->filled_rect_instance_vbo);
        glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(0 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3,4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(5 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(1,1);
        glVertexAttribDivisor(2,1);
        glVertexAttribDivisor(3,1);
    }
    //initialize point vao
    {
        GLuint vbo;
        glGenVertexArrays(1, &rend->point_vao);
        glGenBuffers(1, &rend->point_vbo);
        glBindVertexArray(rend->point_vao);
        glBindBuffer(GL_ARRAY_BUFFER, rend->point_vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)(0 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)(3 * sizeof(float)));
        glVertexAttribDivisor(0,1);
        glVertexAttribDivisor(1,1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        rend->point_alloc_pos= 0;
    }
    //initialize line vao 
    {
        GLuint vbo; //this shouldnt be here????
        glGenVertexArrays(1, &rend->line_vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &rend->line_instance_vbo);
        glBindVertexArray(rend->line_vao);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, rend->line_instance_vbo);
        glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(0 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(0,1);
        glVertexAttribDivisor(1,1);
        glVertexAttribDivisor(2,1);
    }
    //initialize text vao
    {
        GLuint vbo;
        glGenVertexArrays(1, &rend->text_vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &rend->text_instance_vbo);
        glBindVertexArray(rend->text_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(filled_quad_verts), &filled_quad_verts[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, rend->text_instance_vbo);
        glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)(0 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3,2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)(5 * sizeof(float)));
        glVertexAttribDivisor(1,1);
        glVertexAttribDivisor(2,1);
        glVertexAttribDivisor(3,1);
    }
    //initialize billboard vao
    {
        GLuint vbo;
        glGenVertexArrays(1, &rend->billboard_vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &rend->billboard_instance_vbo);
        glBindVertexArray(rend->billboard_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(billboard_data), &billboard_data[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        //this buffer should be updated with the contents of billboard instance data before rendering
        glBindBuffer(GL_ARRAY_BUFFER, rend->billboard_instance_vbo);
        glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(0 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3,2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(7 * sizeof(float)));
        glVertexAttribDivisor(1,1);
        glVertexAttribDivisor(2,1);
        glVertexAttribDivisor(3,1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }


    //initialize forward+ buffers (if forward+ is ok)
    if (1)//rend->renderer_settings.light_cull)
    {
        // Generate our shader storage buffers
        glGenBuffers(1, &rend->light_buffer);
        glGenBuffers(1, &rend->visible_light_indices_buffer);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, rend->light_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, RENDERER_MAX_POINT_LIGHTS* sizeof(PointLight), 0, GL_DYNAMIC_DRAW);

        u32 work_groups_x = (rend->renderer_settings.render_dim.x+ (rend->renderer_settings.render_dim.x% 16)) / 16;
        u32 work_groups_y = (rend->renderer_settings.render_dim.y + (rend->renderer_settings.render_dim.y% 16)) / 16;
        u32 number_of_tiles = work_groups_x* work_groups_y;

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, rend->visible_light_indices_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, number_of_tiles * sizeof(VisibleIndex) * 1024, 0, GL_DYNAMIC_DRAW);



        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, rend->light_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, rend->visible_light_indices_buffer);
        /*
        // Bind light buffer
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_LIGHTS * sizeof(PointLight), 0, GL_DYNAMIC_DRAW);

        // Bind visible light indices buffer
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleLightIndicesBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleIndex) * 1024, 0, GL_STATIC_DRAW);
        */
    }


    shader_load(&rend->shaders[0],"../assets/shaders/blinnphong.vert","../assets/shaders/blinnphong.frag");
    shader_load(&rend->shaders[1],"../assets/shaders/skybox_reflect.vert","../assets/shaders/skybox_reflect.frag");
    shader_load(&rend->shaders[2],"../assets/shaders/postproc.vert","../assets/shaders/postproc.frag");
    shader_load(&rend->shaders[3],"../assets/shaders/shadowmap.vert","../assets/shaders/shadowmap.frag");
    shader_load(&rend->shaders[4],"../assets/shaders/animated3d.vert","../assets/shaders/phong33.frag");
    shader_load(&rend->shaders[5],"../assets/shaders/filled_rect.vert","../assets/shaders/filled_rect.frag");
    shader_load(&rend->shaders[6],"../assets/shaders/line.vert","../assets/shaders/line.frag");
    shader_load(&rend->shaders[7],"../assets/shaders/text.vert","../assets/shaders/text.frag");
    shader_load(&rend->shaders[8],"../assets/shaders/zprepass.vert","../assets/shaders/zprepass.frag");
    shader_load_compute(&rend->shaders[9], "../assets/shaders/light_cull.comp");
    shader_load(&rend->shaders[10],"../assets/shaders/phong33.vert","../assets/shaders/phong33.frag");
    shader_load(&rend->shaders[11],"../assets/shaders/point.vert","../assets/shaders/point.frag");
    shader_load_full(&rend->shaders[12], "../assets/shaders/phong33.vert", "../assets/shaders/red.frag","../assets/shaders/normal_vis.geo");
    shader_load(&rend->shaders[13],"../assets/shaders/phong.vert","../assets/shaders/phong.frag");
    shader_load(&rend->shaders[14],"../assets/shaders/billboard.vert","../assets/shaders/billboard.frag");
    shader_load(&rend->shaders[15],"../assets/shaders/sdf_text.vert","../assets/shaders/sdf_text.frag");
    shader_load(&rend->shaders[16],"../assets/shaders/ssao.vert","../assets/shaders/ssao.frag");
    shader_load(&rend->shaders[17],"../assets/shaders/blur.vert","../assets/shaders/blur.frag");


    //misc
    texture_load(&rend->white_texture,"../assets/white.tga");

    if (rend->renderer_settings.sdf_fonts)
        texture_load(&rend->bmf,"../assets/sdf-font.tga");
    else
        texture_load(&rend->bmf,"../assets/font.tga");

    //generate debuf texture 
    glGenTextures(1, &rend->debug_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rend->debug_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //TODO FIX
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, rend->renderer_settings.render_dim.x,rend->renderer_settings.render_dim.y, 0,  GL_RED, GL_FLOAT, 0);
    glBindImageTexture(0, rend->debug_texture, 0, GL_FALSE, 0,  GL_READ_WRITE, GL_R32F); //maybe its GL_R32F??      
    
}

vec4 global_frustum_corners[FRUSTUM_CORNERS_COUNT];
void renderer_calc_cascades(Renderer * rend, mat4 *light_space_matrix)
{
    f32 near_plane = -0.1f;
    f32 far_plane = -100.f;
    f32 fov = 45.f;
    f32 cascade_end[4] = {near_plane, -20.f, -50.f, -100.f};
    mat4 ortho_proj[RENDERER_CASCADES_COUNT];

    f32 aspect_ratio = rend->renderer_settings.render_dim.x / (f32)rend->renderer_settings.render_dim.y;
    f32 tan_half_vfov = tanf(to_radians(fov/2.f));
    f32 tan_half_hfov = tanf(to_radians(fov * aspect_ratio/2.f));
    mat4 light_matrix = mat4_rotate(70.f, vec3_normalize(v3(1,0.2,0.2)));
    //we find the extents of the frustum by trig functions
    for (u32 c = 0; c < RENDERER_CASCADES_COUNT; ++c)
    {
        f32 f = cascade_end[c+1];
        f32 n = cascade_end[0];
        f32 xn = n * tan_half_hfov;
        f32 xf = f * tan_half_hfov;
        f32 yn = n * tan_half_vfov;
        f32 yf = f * tan_half_vfov; 
                                                    //corners of the near plane (in view space!)
        vec4 frustum_corners[FRUSTUM_CORNERS_COUNT] = {v4(xn,yn,n, 1.f), v4(-xn, yn, n, 1.f), v4(xn,-yn, n, 1.f), v4(-xn, -yn, n, 1.f),
                                                        //corners of the far plane  (in view space!)
                                                        v4(xf,yf,f, 1.f), v4(-xf, yf, f, 1.f), v4(xf, -yf, f, 1.f), v4(-xf, -yf, f, 1.f)};

        //min and max corners (in light space!)
        vec3 min = v3(FLT_MAX, FLT_MAX, FLT_MAX);
        vec3 max = v3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        for (u32 i = 0; i < FRUSTUM_CORNERS_COUNT; ++i)
        {
            //we transform the frustum corners from "view" space to __world__ space
            //vec4 vertex_world = mat4_mulv(inv_view, frustum_corners[i]);
            vec4 vertex_world = mat4_mulv(mat4_inv(rend->view), frustum_corners[i]);
            //we transform the frustum corners from world to light space
            frustum_corners[i] = mat4_mulv(light_matrix, vertex_world);
            global_frustum_corners[i] = frustum_corners[i];
            //index 0 is the cascade start, we don't want that as a cascade end!
            min.x = minimum(min.x, frustum_corners[i].x);
            max.x = maximum(max.x, frustum_corners[i].x);
            min.y = minimum(min.y, frustum_corners[i].y);
            max.y = maximum(max.y, frustum_corners[i].y);
            min.z = minimum(min.z, frustum_corners[i].z);
            max.z = maximum(max.z, frustum_corners[i].z);
        }
        //sprintf(info_log, "[FRUSTUM]min:<%.2f %.2f %.2f>, max:<%.2f %.2f %.2f>", min.x, min.y, min.z, max.x, max.y, max.z);

        //mat4 ortho_proj = orthographic_proj(min.x, max.x, min.y,max.y, min.z, max.z);
        mat4 ortho_proj = orthographic_proj(-50,50,-50,50,-50, 50);
        light_space_matrix[c] = mat4_mul(ortho_proj,light_matrix);

        //calculating clip space cascade ends!
        vec4 vview = v4(0,0,cascade_end[c+1],1);
        //vec4 vclip = mat4_mulv(mat4_mul(rend->proj, rend->view), vview);
        vec4 vclip = mat4_mulv(rend->proj, vview);

        //vclip.z /= vclip.w;
        rend->cascade_ends_clip_space[c] = vclip.z; 
    }


}

void
renderer_begin_frame(Renderer *rend)
{
  rend->proj = perspective_proj(45.f,rend->renderer_settings.render_dim.x / (f32)rend->renderer_settings.render_dim.y, 0.1f,100.f); 
  rend->renderer_settings.render_dim = (ivec2){rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y};

  if (global_platform.window_resized)
  {
      rend->renderer_settings.render_dim.x = global_platform.window_width;
      rend->renderer_settings.render_dim.y = global_platform.window_height;
      fbo_resize(&rend->postproc_fbo, rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0|FBO_COLOR_1|FBO_DEPTH);
      fbo_resize(&rend->main_fbo, rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0|FBO_COLOR_1|FBO_COLOR_2 | FBO_DEPTH);
      if (rend->renderer_settings.ssao_on)
          fbo_resize(&rend->ssao_fbo, rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, FBO_COLOR_0|FBO_DEPTH);

        u32 work_groups_x = (rend->renderer_settings.render_dim.x + (rend->renderer_settings.render_dim.x % 16)) / 16;
        u32 work_groups_y = (rend->renderer_settings.render_dim.y + (rend->renderer_settings.render_dim.y % 16)) / 16;
        u32 number_of_tiles = work_groups_x* work_groups_y;

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, rend->visible_light_indices_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, number_of_tiles * sizeof(VisibleIndex) * 1024, 0, GL_DYNAMIC_DRAW);
  }
  fbo_bind(&rend->postproc_fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0,0,0,0);

  fbo_bind(&rend->ssao_fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0,0,0,0);


  for (u32 i = 0; i < RENDERER_CASCADES_COUNT; ++i)
  {
      fbo_bind(&rend->shadowmap_fbo[i]);
      glClear(GL_DEPTH_BUFFER_BIT);
      //glClearColor(1,1,1,1);
  }



  fbo_bind(&rend->main_fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.7,0.8,1,1);

  //fbo_resize(&rend->depthpeel_fbo, rend->renderer_settings.render_dim.x*2, rend->renderer_settings.render_dim.y*2, FBO_COLOR_0|FBO_DEPTH);
  rend->current_fbo = &rend->main_fbo;
  rend->model_alloc_pos = 0;
  rend->point_alloc_pos= 0;
  rend->animated_model_alloc_pos = 0;
  rend->filled_rect_alloc_pos = 0;
  rend->line_alloc_pos = 0;
  rend->point_light_count = 0;
  rend->text_alloc_pos = 0;
  rend->billboard_alloc_pos = 0;

  //update the camera stuff
  camera_update(&rend->cam);
  rend->view = get_view_mat(&rend->cam);

}

//internal mat4 calc_light_space_matrix(f32 near, f32 far, f32 fov, vec3 dlp, mat4 view, mat4 world) {return m4d(1.f);}

internal void
renderer_set_light_uniforms(Renderer *rend, Shader *s)
{
    shader_set_int(s, "point_light_count", rend->point_light_count);
    for (i32 i = 0; i < rend->point_light_count;++i)
    {
      if (i < 10)
      {
        point_attr[0][13] = '0'+i;
        point_attr[1][13] = '0'+i;
        point_attr[2][13] = '0'+i;
        point_attr[3][13] = '0'+i;
        shader_set_vec3(s,point_attr[0], rend->point_lights[i].position);
        shader_set_vec3(s,point_attr[1], rend->point_lights[i].ambient);
        shader_set_vec3(s,point_attr[2], rend->point_lights[i].diffuse);
        shader_set_vec3(s,point_attr[3], rend->point_lights[i].specular);
      }
      else
      {
        big_point_attr[0][13] = '0'+ (i / 10);
        big_point_attr[0][14] = '0'+ (i % 10);
        big_point_attr[1][13] = '0'+ (i / 10);
        big_point_attr[1][14] = '0'+ (i % 10);
        big_point_attr[2][13] = '0'+ (i / 10);
        big_point_attr[2][14] = '0'+ (i % 10);
        big_point_attr[3][13] = '0'+ (i / 10);
        big_point_attr[3][14] = '0'+ (i % 10);
        shader_set_vec3(s,big_point_attr[0], rend->point_lights[i].position);
        shader_set_vec3(s,big_point_attr[1], rend->point_lights[i].ambient);
        shader_set_vec3(s,big_point_attr[2], rend->point_lights[i].diffuse);
        shader_set_vec3(s,big_point_attr[3], rend->point_lights[i].specular);
      }
    }
    //directional light properties
    shader_set_vec3(s, "dirlight.direction", rend->directional_light.direction);
    shader_set_vec3(s, "dirlight.ambient", rend->directional_light.ambient);
    shader_set_vec3(s, "dirlight.diffuse", rend->directional_light.diffuse);
    shader_set_vec3(s, "dirlight.specular", rend->directional_light.specular);



}
internal void
renderer_set_ssao_kernel_uniforms(Renderer *rend, Shader *s, vec3 *ssao_kernel)
{
    for (i32 i = 0; i < 64;++i)
    {
      if (i < 10)
      {
        kernel_attr[7] = '0'+i;
        shader_set_vec3(s,kernel_attr, ssao_kernel[i]);
      }
      else
      {
        big_kernel_attr[7] = '0'+ (i / 10);
        big_kernel_attr[8] = '0'+ (i % 10);
        shader_set_vec3(s,big_kernel_attr, ssao_kernel[i]);
      }
    }
}

//here we put the draw calls for everything that needs shadowmapping!
renderer_render_scene3D(Renderer *rend,Shader *shader)
{
  mat4 inv_view = mat4_inv(rend->view);
  vec3 view_pos = v3(inv_view.elements[3][0],inv_view.elements[3][1],inv_view.elements[3][2]);
  renderer_push_line(rend, v3(0,0,0), rend->directional_light.direction, v4(0.4f,0.4f,0.9f,1.f));
  for(i32 i = 0; i < rend->model_alloc_pos;++i)
  { 
    RendererModelData data = rend->model_instance_data[i];

#if 0
    mat4 ortho_proj = orthographic_proj(-20.f, 20.f, -20.f, 20.f, -20.f, 140.f);
    vec3 pos = rend->cam.pos;
    //light_space_matrix = mat4_mul(ortho_proj,look_at(dir_light_pos, vec3_normalize(vec3_add(rend->directional_light.direction, dir_light_pos)), v3(0,1,0)));
    light_space_matrix[0] = mat4_mul(ortho_proj,mat4_rotate(90.f, v3(1,0,0)));

    sprintf(info_log, "pos:<%.2f %.2f %.2f>", rend->cam.pos.y, rend->cam.pos.y, rend->cam.pos.z);
#else
#endif

    if (!rend->renderer_settings.light_cull)
        renderer_set_light_uniforms(rend, shader);


    use_shader(&shader[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, data.diff->id);
    shader_set_int(&shader[0], "material.diffuse_map", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, data.spec->id);
    shader_set_int(&shader[0], "material.specular_map", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, data.bump->id);
    shader_set_int(&shader[0], "material.bump_map", 2);



    //in case we need the skybox's texture for the rendering
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, rend->skybox.tex_id);
    shader_set_int(&rend->skybox.shader, "skybox", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, rend->shadowmap_fbo[0].depth_attachment);
    shader_set_int(&shader[0], "shadow_map[0]", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, rend->shadowmap_fbo[1].depth_attachment);
    shader_set_int(&shader[0], "shadow_map[1]", 5);


    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, rend->shadowmap_fbo[2].depth_attachment);
    shader_set_int(&shader[0], "shadow_map[2]", 6);


    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, rend->ssao_fbo.color_attachments[0]);
    shader_set_int(&shader[0], "ssao_texture", 7);

    shader_set_mat4fv(&shader[0], "model", (GLfloat*)data.model.elements);
    shader_set_mat4fv(&shader[0], "view", (GLfloat*)rend->view.elements);
    shader_set_mat4fv(&shader[0], "proj", (GLfloat*)rend->proj.elements);
    shader_set_mat4fv(&shader[0], "lsm", (GLfloat*)rend->active_lsm.elements);//this is the one used for depth map rendering (shadowmap.vert)!!
    shader_set_mat4fv(&shader[0], "light_space_matrix[0]", (GLfloat*)rend->lsms[0].elements);
    shader_set_mat4fv(&shader[0], "light_space_matrix[1]", (GLfloat*)rend->lsms[1].elements);
    shader_set_mat4fv(&shader[0], "light_space_matrix[2]", (GLfloat*)rend->lsms[2].elements);
    shader_set_float(&shader[0], "cascade_ends_clip_space[0]", rend->cascade_ends_clip_space[0]);
    shader_set_float(&shader[0], "cascade_ends_clip_space[1]", rend->cascade_ends_clip_space[1]);
    shader_set_float(&shader[0], "cascade_ends_clip_space[2]", rend->cascade_ends_clip_space[2]);
    shader_set_vec3(&shader[0], "view_pos", view_pos);
    shader_set_int(&shader[0], "window_width", global_platform.window_width);
    shader_set_int(&shader[0], "window_height", global_platform.window_height);
    //set material properties
    shader_set_float(&shader[0], "material.shininess", data.material->shininess);
    shader_set_int(&shader[0], "material.has_diffuse_map", data.material->has_diffuse_map);
    shader_set_int(&shader[0], "material.has_specular_map", data.material->has_specular_map);
    shader_set_int(&shader[0], "material.has_bump_map", data.material->has_bump_map);
    shader_set_vec3(&shader[0], "material.ambient", data.material->ambient);
    shader_set_vec3(&shader[0], "material.diffuse", data.material->diffuse);
    shader_set_vec3(&shader[0], "material.specular", data.material->specular);
    shader_set_vec3(&shader[0], "material.emmisive", data.material->emmisive);
    //light properties
    //renderer_set_light_uniforms(rend, shader);
    i32 work_groups_x = (rend->renderer_settings.render_dim.x + (rend->renderer_settings.render_dim.x % 16)) / 16;
    shader_set_int(&shader[0], "number_of_tiles_x", work_groups_x);
    shader_set_vec3(&shader[0], "dirlight.direction", rend->directional_light.direction);
    shader_set_vec3(&shader[0], "dirlight.ambient", rend->directional_light.ambient);
    shader_set_vec3(&shader[0], "dirlight.diffuse", rend->directional_light.diffuse);
    shader_set_vec3(&shader[0], "dirlight.specular", rend->directional_light.specular);



    glBindVertexArray(data.model_vao);
    glDrawArrays(GL_TRIANGLES,0, data.model_vertex_count);
    //glDrawArrays(GL_LINES,0, data.model_vertex_count);
    glBindVertexArray(0);
  }

}

internal void renderer_check_gl_errors(void)
{
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
       sprintf(error_log, "GL error: %i", err);
    }

}

void
renderer_end_frame(Renderer *rend)
{
  mat4 inv_view = mat4_inv(rend->view);
  vec3 view_pos = v3(inv_view.elements[3][0],inv_view.elements[3][1],inv_view.elements[3][2]);
  //renderer_check_gl_errors();
  
#if 0
  for (int i = 0; i < FRUSTUM_CORNERS_COUNT;++i)
  {
      rend->test_sphere.model = mat4_translate(v3(global_frustum_corners[i].x,global_frustum_corners[i].y,global_frustum_corners[i].z));
      renderer_push_model(rend, &rend->test_sphere);
  }
#endif

  glDisable(GL_BLEND);

  //set light ssbo @check
  {
   	glBindBuffer(GL_SHADER_STORAGE_BUFFER, rend->light_buffer);
	PointLight *point_lights = (PointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
    memcpy(point_lights, rend->point_lights, sizeof(PointLight) * rend->point_light_count);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); 
  }
  //first, we render the scene to the depth map
  fbo_bind(&rend->shadowmap_fbo[0]);
  rend->active_lsm = rend->lsms[0];
  renderer_render_scene3D(rend,&rend->shaders[3]);

  //calculate cascades for the RENDERER_CASCADES_COUNT shadow maps! 
  renderer_calc_cascades(rend, rend->lsms);

  if (rend->renderer_settings.cascaded_render)
  {
      fbo_bind(&rend->shadowmap_fbo[1]);
      rend->active_lsm = rend->lsms[1];
      renderer_render_scene3D(rend,&rend->shaders[3]);
      rend->active_lsm = rend->lsms[2];
      fbo_bind(&rend->shadowmap_fbo[2]);
      renderer_render_scene3D(rend,&rend->shaders[3]);
  }

  //update instance data for filled rect
  glBindBuffer(GL_ARRAY_BUFFER, rend->filled_rect_instance_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(RendererFilledRect) * rend->filled_rect_alloc_pos, &rend->filled_rect_instance_data[0], GL_DYNAMIC_DRAW);
  //update instance data for line 
  glBindBuffer(GL_ARRAY_BUFFER, rend->line_instance_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(RendererLine) * rend->line_alloc_pos, &rend->line_instance_data[0], GL_DYNAMIC_DRAW);
  //update instance data for text 
  glBindBuffer(GL_ARRAY_BUFFER, rend->text_instance_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(RendererChar) * rend->text_alloc_pos, &rend->text_instance_data[0], GL_DYNAMIC_DRAW);
  //update point instance data
  glBindBuffer(GL_ARRAY_BUFFER, rend->point_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(RendererPointData) * rend->point_alloc_pos, &rend->point_instance_data[0], GL_DYNAMIC_DRAW);
  //update point instance data
  glBindBuffer(GL_ARRAY_BUFFER, rend->billboard_instance_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(RendererBillboard) * rend->billboard_alloc_pos, &rend->billboard_instance_data[0], GL_DYNAMIC_DRAW);




  fbo_bind(&rend->main_fbo);
  if (rend->renderer_settings.z_prepass)
  {
      glDepthFunc(GL_LESS);
      //glColorMask(0,0,0,0);
      glDepthMask(GL_TRUE);

      //glDisable(GL_BLEND);
      renderer_render_scene3D(rend,&rend->shaders[8]);
      glDepthFunc(GL_LEQUAL);
      glColorMask(1,1,1,1);
      //glEnable(GL_BLEND);

      //we clear color attachment 1 (we need it to write bright colors!)
      //glClearTexImage(rend->main_fbo.color_attachments[1], 0, GL_RGBA, GL_FLOAT, 0); 
  }


  //render ssao texture
  if (rend->renderer_settings.ssao_on)
  {
        vec3 ssao_kernel[64];
        for(u32 i = 0; i< 64; ++i)
        {
            //Note: these are TBN space coordinates!
            vec3 sample = v3(random01() * 2.f - 1.f, random01() * 2.f - 1.f, random01());
            sample = vec3_normalize(sample);
            sample = vec3_mulf(sample, random01());
            f32 scale = i / 64.f;
            scale = lerp(0.1f, 1.f, scale * scale); //making samples closer to origin
            sample = vec3_mulf(sample, scale);
            ssao_kernel[i] = sample;
        }
        //now we calculate the SSAO texture, we put it in the ssao fbo using depth + normals from main fbo
        //glBindFramebuffer(GL_FRAMEBUFFER, rend->ssao_fbo.fbo);
        fbo_bind(&rend->ssao_fbo);
        glBindVertexArray(rend->postproc_vao);
        use_shader(&rend->shaders[16]);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, rend->main_fbo.color_attachments[2]);
        shader_set_int(&rend->shaders[16],"normal_texture",2);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, rend->main_fbo.color_attachments[1]);
        shader_set_int(&rend->shaders[16],"position_texture",1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rend->main_fbo.depth_attachment);
        shader_set_int(&rend->shaders[16],"depth_texture",0);
        shader_set_mat4fv(&rend->shaders[16], "proj", (GLfloat*)rend->proj.elements);
        shader_set_mat4fv(&rend->shaders[16], "view", (GLfloat*)rend->view.elements);
        mat4 inv_view = mat4_inv(rend->view);
        vec3 view_pos = v3(inv_view.elements[3][0],inv_view.elements[3][1],inv_view.elements[3][2]);
        shader_set_vec3(&rend->shaders[16], "view_pos", view_pos); 

        renderer_set_ssao_kernel_uniforms(rend, &rend->shaders[16], ssao_kernel);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //after rendering ssao we blur it a bit :)
        use_shader(&rend->shaders[17]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rend->ssao_fbo.color_attachments[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        shader_set_int(&rend->shaders[16],"screenTexture",0);


        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, rend->main_fbo.fbo);
  }

  //launch compute shader for light culling
  {
      glMemoryBarrier(GL_ALL_BARRIER_BITS);
      u32 work_groups_x = (rend->renderer_settings.render_dim.x+ (rend->renderer_settings.render_dim.x % 16)) / 16;
      u32 work_groups_y = (rend->renderer_settings.render_dim.y + (rend->renderer_settings.render_dim.y % 16)) / 16;

      glMemoryBarrier(GL_ALL_BARRIER_BITS);
      use_shader(&rend->shaders[9]);
      shader_set_int(&rend->shaders[9], "window_width", rend->renderer_settings.render_dim.x);
      shader_set_mat4fv(&rend->shaders[9], "proj", (GLfloat*)rend->proj.elements);
      shader_set_mat4fv(&rend->shaders[9], "view", (GLfloat*)rend->view.elements);
      shader_set_int(&rend->shaders[9], "window_height", rend->renderer_settings.render_dim.y);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, rend->main_fbo.depth_attachment);
      shader_set_int(&rend->shaders[9], "depth_map", 0);
      shader_set_int(&rend->shaders[9], "point_light_count", rend->point_light_count);

      glDispatchCompute((GLuint)work_groups_x, (GLuint)work_groups_y, 1);
      //synchronize everything
      glMemoryBarrier(GL_ALL_BARRIER_BITS);
  }


  //then we render to the main fbo
  //TODO TODO TODO this should go inside render scene 3d!!!!!!!!!!!!!!!!!!!!!!!
  for (u32 i = 0; i < rend->animated_model_alloc_pos; ++i)
  {
      use_shader(&rend->shaders[4]);
    
    shader_set_mat4fv(&rend->shaders[4], "proj", (GLfloat*)rend->proj.elements);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rend->animated_model_instance_data[i].diff.id);
    shader_set_int(&rend->shaders[4], "material.diffuse", 0); //we should really make the texture manager global or something(per Scene?)... sigh
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, rend->animated_model_instance_data[i].spec.id);
    shader_set_int(&rend->shaders[4], "material.specular", 1);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, rend->shadowmap_fbo[0].depth_attachment);
    shader_set_int(&rend->shaders[4], "shadow_map", 4);

    if (0){
        mat4 identity = m4d(1.f);
        shader_set_mat4fv(&rend->shaders[4], "joint_transforms[0]", (GLfloat*)identity.elements);
        shader_set_mat4fv(&rend->shaders[4], "joint_transforms[1]", (GLfloat*)identity.elements);
        shader_set_mat4fv(&rend->shaders[4], "joint_transforms[2]", (GLfloat*)identity.elements);
        shader_set_mat4fv(&rend->shaders[4], "joint_transforms[3]", (GLfloat*)identity.elements);
        shader_set_mat4fv(&rend->shaders[4], "joint_transforms[4]", (GLfloat*)identity.elements);
        shader_set_mat4fv(&rend->shaders[4], "joint_transforms[5]", (GLfloat*)identity.elements);
        shader_set_mat4fv(&rend->shaders[4], "joint_transforms[6]", (GLfloat*)identity.elements);
        shader_set_mat4fv(&rend->shaders[4], "joint_transforms[7]", (GLfloat*)identity.elements);
        shader_set_mat4fv(&rend->shaders[4], "joint_transforms[8]", (GLfloat*)identity.elements);
        shader_set_mat4fv(&rend->shaders[4], "joint_transforms[9]", (GLfloat*)identity.elements);
        //@memleak
        char str[32] = "joint_transforms[xx]";

        for (i32 i = 10; i < rend->animated_model_instance_data[i].joint_count; ++i)
        {
            str[17] = '0' + (i/10);
            str[18] = '0' + (i -(((int)(i/10)) * 10));
            shader_set_mat4fv(&rend->shaders[4], str, (GLfloat*)identity.elements);
        }
    }
    for (u32 j = 0; j < rend->animated_model_instance_data[i].joint_count; ++j)
        set_joint_transform_uniforms(&rend->shaders[4], &rend->animated_model_instance_data[i].joints[j]);
    shader_set_mat4fv(&rend->shaders[4], "view", (GLfloat*)rend->view.elements);
    shader_set_mat4fv(&rend->shaders[4], "model", (GLfloat*)rend->animated_model_instance_data[i].model.elements);
    shader_set_vec3(&rend->shaders[4], "lightdir", rend->directional_light.direction); 
    renderer_set_light_uniforms(rend, &rend->shaders[4]);
    mat4 inv_view = mat4_inv(rend->view);
    vec3 view_pos = v3(inv_view.elements[3][0],inv_view.elements[3][1],inv_view.elements[3][2]);
    shader_set_vec3(&rend->shaders[4], "view_pos", view_pos); 
    //mat4 ortho_proj = orthographic_proj(-200.f, 200.f, -200.f, 200.f, 0.01f, 200.f);
    //mat4 light_space_matrix = mat4_mul(ortho_proj,look_at(v3(0,100,0), v3(10,0,0), v3(0,1,0)));
    //shader_set_mat4fv(&rend->shaders[4], "light_space_matrix", (GLfloat*)light_space_matrix.elements);




    glBindVertexArray(rend->animated_model_instance_data[i].vao);
    glDrawArrays(GL_TRIANGLES,0, rend->animated_model_instance_data[i].vertices_count);
    //glDrawArrays(GL_LINES,0, 40000);
    glBindVertexArray(0);
  }
  //update instance data for line 
  glBindBuffer(GL_ARRAY_BUFFER, rend->line_instance_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(RendererLine) * rend->line_alloc_pos, &rend->line_instance_data[0], GL_DYNAMIC_DRAW);

    //render lines
    glLineWidth(5);
    use_shader(&rend->shaders[6]);
    mat4 mvp = mat4_mul(rend->proj, rend->view);
    shader_set_mat4fv(&rend->shaders[6], "MVP", (GLfloat*)mvp.elements);
    glBindVertexArray(rend->line_vao);
    glDrawArraysInstanced(GL_LINES, 0, 2, rend->line_alloc_pos);
    glBindVertexArray(0);
    //render points
    glPointSize(10);
   use_shader(&rend->shaders[11]);
   shader_set_mat4fv(&rend->shaders[11], "view", (GLfloat*)rend->view.elements);
   shader_set_mat4fv(&rend->shaders[11], "proj", (GLfloat*)rend->proj.elements);
   glBindVertexArray(rend->point_vao);
   glDrawArraysInstanced(GL_POINTS, 0, 1, rend->point_alloc_pos);
   glBindVertexArray(0);

    //render billboards 
    use_shader(&rend->shaders[14]);
    shader_set_mat4fv(&rend->shaders[14], "view", (GLfloat*)rend->view.elements);
    shader_set_mat4fv(&rend->shaders[14], "proj", (GLfloat*)rend->proj.elements);
    shader_set_vec3(&rend->shaders[14], "cam_up", v3(rend->view.elements[0][0],rend->view.elements[1][0],rend->view.elements[2][0]));
    shader_set_vec3(&rend->shaders[14], "cam_right", v3(rend->view.elements[0][1],rend->view.elements[1][1],rend->view.elements[2][1]));
    glBindVertexArray(rend->billboard_vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, rend->billboard_alloc_pos);
    glBindVertexArray(0);



   //if we are in debug mode, draw model normals!!
      
   if (rend->renderer_settings.debug_mode)
       renderer_render_scene3D(rend,&rend->shaders[12]);

   if (rend->renderer_settings.light_cull) 
       renderer_render_scene3D(rend,&rend->shaders[0]);
   else
   {
      renderer_set_light_uniforms(rend, &rend->shaders[10]);
      shader_set_int(&rend->shaders[10], "point_light_count", rend->point_light_count);
      renderer_render_scene3D(rend,&rend->shaders[10]);
   }
  glEnable(GL_BLEND);
  skybox_render(&rend->skybox, rend->proj, rend->view);


  glBindFramebuffer(GL_FRAMEBUFFER, rend->postproc_fbo.fbo);
  glBindTexture(GL_TEXTURE_2D, rend->main_fbo.color_attachments[0]);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, rend->renderer_settings.render_dim.x, rend->renderer_settings.render_dim.y);

    glEnable(GL_DEPTH_TEST);
    use_shader(&rend->shaders[2]);
    glBindVertexArray(rend->postproc_vao);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, rend->main_fbo.color_attachments[0]);
    shader_set_int(&rend->shaders[2],"screenTexture",1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, rend->main_fbo.color_attachments[1]);
    shader_set_int(&rend->shaders[2],"brightTexture",2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rend->main_fbo.depth_attachment);
    shader_set_int(&rend->shaders[2],"depthTexture",0);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, rend->ssao_fbo.color_attachments[0]);
    shader_set_int(&rend->shaders[2],"ssao_texture",3);
    shader_set_mat4fv(&rend->shaders[2], "proj", (GLfloat*)rend->proj.elements);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0,rend->renderer_settings.render_dim.x,rend->renderer_settings.render_dim.y);


    //fbo_copy_contents(rend->main_fbo.fbo,0);
    fbo_copy_contents(rend->postproc_fbo.fbo,0);
    //render filled rects
    glDisable(GL_DEPTH_TEST);
    use_shader(&rend->shaders[5]);
    shader_set_mat4fv(&rend->shaders[5], "view", (GLfloat*)rend->view.elements);
    glBindVertexArray(rend->filled_rect_vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, rend->filled_rect_alloc_pos);
    glBindVertexArray(0);




    //at the end we render the skybox

    u32 font_shader_index = rend->renderer_settings.sdf_fonts ? 15 : 7;

    use_shader(&rend->shaders[font_shader_index]);
    shader_set_mat4fv(&rend->shaders[font_shader_index], "view", (GLfloat*)rend->view.elements);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rend->bmf.id);
    shader_set_int(&rend->shaders[font_shader_index], "bmf_sampler",0);
    glBindVertexArray(rend->text_vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4,rend->text_alloc_pos);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

}

void renderer_push_model(Renderer *rend, Model *m)
{
  RendererModelData data = (RendererModelData){0};
  for (u32 i = 0; i < m->mesh_count; ++i)
  {
    data.model = m->model;
    data.model_vao = m->meshes[i].vao;
    data.model_vertex_count = m->meshes[i].vertices_count;
    data.diff = &m->meshes[i].material.diff;
    data.spec = &m->meshes[i].material.spec;
    data.bump = &m->meshes[i].material.bump;
    data.material = &m->meshes[i].material;
    rend->model_instance_data[rend->model_alloc_pos++] = data;
  }

}
/*
typedef struct RendererAnimatedModelData
{
    GLuint vao;
    Texture diff_tex;
    Texture spec_tex;
    
    u32 joint_count;
    Joint *joints;
    u32 vertices_count;
}RendererAnimatedModelData;
*/

void renderer_push_animated_model(Renderer *rend, AnimatedModel *m)
{
  RendererAnimatedModelData data = {0};
  data.vao = m->vao;
  data.diff = *m->diff_tex;
  data.spec = rend->white_texture; //declare a white texture for such cases (in renderer)
  data.joint_count = m->joint_count;
  data.joints = m->joints;
  data.vertices_count = m->vertices_count; 
  data.model = m->model;
  rend->animated_model_instance_data[rend->animated_model_alloc_pos++] = data;
}

void renderer_push_filled_rect(Renderer *rend, vec3 pos, vec2 dim, vec4 color)
{
    RendererFilledRect rect = (RendererFilledRect){pos, dim, color};
    rend->filled_rect_instance_data[rend->filled_rect_alloc_pos++] = rect;
}

void renderer_push_text(Renderer *rend, vec3 pos, vec2 dim, char *str)
{
    u32 str_len = str_size(str);
    for (u32 ch = 0; ch < str_len;++ch)
    {
        char letter = str[ch];
        f32 uv_x = (letter % 16) / 16.f;
        f32 uv_y = 1 - (letter / 16) /16.f - 1.f/16.f;
        vec2 uv_down_left = v2(uv_x, uv_y);
        //for each char in string .. put 'em in the instance data
        pos.x += dim.x / 2.f;
        RendererChar c = (RendererChar){pos, dim, uv_down_left}; //@Fix
        rend->text_instance_data[rend->text_alloc_pos++] = c;
    }
}

void renderer_push_char(Renderer *rend, vec3 pos, vec2 dim, char ch)
{
    char letter = ch;
    f32 uv_x = (letter % 16) / 16.f;
    f32 uv_y = 1 - (letter / 16) /16.f - 1.f/16.f;
    vec2 uv_down_left = v2(uv_x, uv_y);
    //for each char in string .. put 'em in the instance data
    pos.x += dim.x;
    RendererChar c = (RendererChar){pos, vec2_mulf(dim, 1.5), uv_down_left}; //@Fix
    rend->text_instance_data[rend->text_alloc_pos++] = c;
}


void renderer_push_line(Renderer *rend, vec3 start, vec3 end, vec4 color)
{
    RendererLine line = (RendererLine){start,end,color};
    rend->line_instance_data[rend->line_alloc_pos++] = line;
}


void renderer_push_point_light(Renderer *rend, PointLight l)
{
    rend->point_lights[rend->point_light_count++] = l;
}
void renderer_push_compass(Renderer *rend, vec3 position)
{
    renderer_push_line(rend, position, vec3_add(position, v3(0,2,0)), v4(0.2,0.2,0.9, 1.0));
    renderer_push_line(rend, position, vec3_add(position, v3(2,0,0)), v4(0.9,0.2,0.2, 1.0));
    renderer_push_line(rend, position, vec3_add(position, v3(0,0,-2)), v4(0.2,0.9,0.2, 1.0));
}

void renderer_push_cube_wireframe(Renderer *rend, vec3 min, vec3 max)
{
    renderer_push_line(rend, min, v3(max.x,min.y,min.z), v4(0.9,0.2,0.2,1.f));
    renderer_push_line(rend, min, v3(min.x,max.y,min.z), v4(0.9,0.2,0.2,1.f));
    renderer_push_line(rend, min, v3(min.x,min.y,max.z), v4(0.9,0.2,0.2,1.f));
    renderer_push_line(rend, max, v3(min.x,max.y,max.z), v4(0.9,0.2,0.2,1.f));
    renderer_push_line(rend, max, v3(max.x,min.y,max.z), v4(0.9,0.2,0.2,1.f));
    renderer_push_line(rend, max, v3(max.x,max.y,min.z), v4(0.9,0.2,0.2,1.f));
    renderer_push_line(rend, v3(min.x,min.y,max.z), v3(max.x,min.y,max.z), v4(0.9,0.2,0.2,1.f));
    renderer_push_line(rend, v3(min.x,min.y,max.z), v3(min.x,max.y,max.z), v4(0.9,0.2,0.2,1.f));
    renderer_push_line(rend, v3(min.x, max.y,min.z), v3(min.x,max.y,max.z), v4(0.9,0.2,0.2,1.f));
    renderer_push_line(rend, v3(min.x,max.y,min.z), v3(max.x,max.y,min.z), v4(0.9,0.2,0.2,1.f));
    renderer_push_line(rend, v3(max.x,max.y,min.z), v3(max.x,min.y,min.z), v4(0.9,0.2,0.2,1.f));
    renderer_push_line(rend, v3(max.x,min.y,min.z),v3(max.x,min.y,max.z), v4(0.9,0.2,0.2,1.f));
}

//                                              obb center   rot bases  halfwidths 
void renderer_push_obb_wireframe(Renderer *rend, vec3 center, f32 *u, vec3 e)
{
    mat4 rotation_matrix = m4d(1.f);
    u32 base_index= 0;

    for (u32 i = 0; i < 3; ++i)
        for (u32 j = 0; j < 3; ++j)
            rotation_matrix.elements[i][j] = u[base_index++];

    vec3 verts[30] = {
        v3(e.x,-e.y,-e.z), v3(e.x,e.y,-e.z), v3(e.x,e.y,e.z), v3(e.x,-e.y,e.z),v3(e.x,-e.y,-e.z),
        v3(-e.x,-e.y,-e.z), v3(-e.x,e.y,-e.z), v3(-e.x,e.y,e.z), v3(-e.x,-e.y,e.z),v3(-e.x,-e.y,-e.z),

        v3(-e.x,e.y,-e.z), v3(e.x,e.y,-e.z), v3(e.x,e.y,e.z), v3(-e.x,e.y,e.z),v3(-e.x,e.y,-e.z),
        v3(-e.x,-e.y,-e.z), v3(e.x,-e.y,-e.z), v3(e.x,-e.y,e.z), v3(-e.x,-e.y,e.z),v3(-e.x,-e.y,-e.z),

        v3(-e.x,-e.y,e.z), v3(e.x,-e.y,e.z), v3(e.x,e.y,e.z), v3(-e.x,e.y,e.z),v3(-e.x,-e.y,e.z),
        v3(-e.x,-e.y,-e.z), v3(e.x,-e.y,-e.z), v3(e.x,e.y,-e.z), v3(-e.x,e.y,-e.z),v3(-e.x,-e.y,-e.z),
    };
    for (u32 i = 0; i < 30; ++i)
    {
        vec4 local_pos = v4(verts[i].x, verts[i].y, verts[i].z,1.f);
        vec4 rotated_local_pos = mat4_mulv(rotation_matrix, local_pos);
        vec3 global_pos = vec3_add(center, v3(rotated_local_pos.x, rotated_local_pos.y, rotated_local_pos.z));
        verts[i] = global_pos;
    }
    for (u32 i = 1; i < 30; ++i)
    {
        renderer_push_line(rend, verts[i-1], verts[i], v4(0.2,0.4,0.6,1.f));
    }
}

void renderer_push_quad_wireframe(Renderer *rend, vec3 v0, vec3 v1, vec3 v2, vec3 v3)
{
    renderer_push_line(rend, v0, v1, v4(0.8,0.2,0.2,1.f));
    renderer_push_line(rend, v1, v2, v4(0.8,0.2,0.2,1.f));
    renderer_push_line(rend, v2, v3, v4(0.8,0.2,0.2,1.f));
    renderer_push_line(rend, v3, v0, v4(0.8,0.2,0.2,1.f));
}



void renderer_push_point(Renderer *rend, RendererPointData point)
{
    rend->point_instance_data[rend->point_alloc_pos++] = point;

}

void renderer_push_billboard(Renderer *rend, vec3 center, vec4 color)
{
    RendererBillboard b;
    b.center = center;
    b.color = color;
    b.dim = v2(0.05, 0.05);
    rend->billboard_instance_data[rend->billboard_alloc_pos++] = b;
}

