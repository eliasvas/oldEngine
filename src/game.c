#include "platform.h"
#include "tools.h"
#include "objloader.h"
#include "camera.h"
#include "model.h"
#include "quad.h" 
#include "fbo.h"
#include "renderer.h"
mat4 view,proj;

global Camera cam;
global Model debug_cube;
global Model light_cube;
global Model arena;
global Renderer rend;

internal void 
init(void)
{
    camera_init(&cam);
    model_init_cube(&debug_cube);
    renderer_init(&rend);
    model_init_cube(&light_cube);
    light_cube.diff = debug_cube.spec;


    
    MeshInfo arena_mesh = obj_load("../assets/arena/arena.obj");
    model_init(&arena, &arena_mesh);
}



internal void 
update(void)
{
  renderer_begin_frame(&rend);
  camera_update(&cam);
  rend.proj = perspective_proj(45.f,global_platform.window_width / (f32)global_platform.window_height, 0.1f,300.f); 
  rend.view = get_view_mat(&cam);
}

internal void 
render(void)
{
    //renderer_push_point_light(&rend,(PointLight){v3(40*sin(global_platform.current_time),5,40*cos(global_platform.current_time)),
     //   1.f,0.09f,0.0032f,v3(0.6,0.5,0.7),v3(0.9,0.8,0.8),v3(0.9,0.8,0.8),256.f});

    light_cube.position = v3(40*sin(global_platform.current_time),5,40*cos(global_platform.current_time));
    renderer_push_model(&rend, &light_cube);



    renderer_push_model(&rend, &arena);
    renderer_end_frame(&rend);
}

