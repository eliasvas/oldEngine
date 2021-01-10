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
global Model model;
global Renderer rend;

internal void 
init(void)
{
    camera_init(&cam);
    model_init_cube(&debug_cube);
    renderer_init(&rend);
    model_init_cube(&light_cube);
    light_cube.meshes[0].material.diff = debug_cube.meshes[0].material.spec;

    model = model_info_init("../assets/arena/arena.mtl");

    
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
    renderer_push_point_light(&rend,(PointLight){v3(40*sin(global_platform.current_time),5,40*cos(global_platform.current_time)),
        1.f,0.09f,0.0032f,v3(6,5,7),v3(9,8,8),v3(9,8,8),256.f});

    /*
    for (int i = 0; i < 10; ++i)
    {
      for(int j = 0; j < 10;++j)
      {
        debug_cube.position = v3(2*i,i*sin(global_platform.current_time),2*j);
        renderer_push_model(&rend, &debug_cube);
      }
    }
    */

    light_cube.model = mat4_translate(v3(40*sin(global_platform.current_time),5,40*cos(global_platform.current_time)));
    renderer_push_model(&rend, &light_cube);



    renderer_push_model(&rend,&model);
    renderer_end_frame(&rend);
}

