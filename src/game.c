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
global Renderer rend;

internal void 
init(void)
{
    camera_init(&cam);
    model_init_cube(&debug_cube);
    renderer_init(&rend);
}



internal void 
update(void)
{
  renderer_begin_frame(&rend);
  camera_update(&cam);
  rend.proj = perspective_proj(45.f,global_platform.window_width / (f32)global_platform.window_height, 0.1f,100.f); 
  rend.view = get_view_mat(&cam);
}

internal void 
render(void)
{
    for (i32 i = 0; i < 20; i+=2)
    {
      for (i32 j = 0; j < 20;j+=2)
      {
        debug_cube.position = v3(i + 0.001,sin(i * global_platform.current_time),-j);
        renderer_push_model(&rend, &debug_cube);
      }
    }


    renderer_push_point_light(&rend,(PointLight){v3(10*cos(global_platform.current_time*4),10*sin(global_platform.current_time),sin(global_platform.current_time * 3.4f)),
        1.f,0.09f,0.0032f,v3(0.8,0.8,0.8),v3(0.9,0.8,0.8),v3(1,0.9,0.8),256.f});

    debug_cube.position = v3(10*cos(global_platform.current_time*4),10*sin(global_platform.current_time),sin(global_platform.current_time * 3.4f));
    renderer_push_model(&rend, &debug_cube);
    renderer_end_frame(&rend);
}

