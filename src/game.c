#include "platform.h"
#include "tools.h"
#include "objloader.h"
#include "camera.h"
#include "model.h"
#include "quad.h" 
#include "fbo.h"
#include "renderer.h"
#include "collada_parser.h"
#include "animation.h"
#include "entity.h"
mat4 view,proj;

global Camera cam;
global Model debug_cube;
global Model light_cube;
global Model model;
global Renderer rend;
global Animator animator;

global b32 UI_OPEN;
global f32 trans = 0.f;

internal void 
init(void)
{
    camera_init(&cam);
    model_init_cube(&debug_cube);
    renderer_init(&rend);
    model_init_cube(&light_cube);
    light_cube.meshes[0].material.diff = debug_cube.meshes[0].material.spec;

    model = model_info_init("../assets/arena/arena.mtl");
    model.model = mat4_scale(v3(0.2f,0.2f,0.2f));
    animator = animator_init(str(&global_platform.frame_storage,"../assets/bender/bender.tga"), 
        str(&global_platform.frame_storage,"../assets/bender/bender.dae"), str(&global_platform.frame_storage,"../assets/bender/bender.dae"));  
}



internal void 
update(void)
{
  renderer_begin_frame(&rend);
  camera_update(&cam);
  rend.proj = perspective_proj(45.f,global_platform.window_width / (f32)global_platform.window_height, 0.1f,80.f); 
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
        debug_cube.model = mat4_translate(v3(2*i,i*sin(global_platform.current_time),2*j));
        renderer_push_model(&rend, &debug_cube);
      }
    }
    */

    light_cube.model = mat4_translate(v3(40*sin(global_platform.current_time),5,40*cos(global_platform.current_time)));
    renderer_push_model(&rend, &light_cube);

    //UI bullshit..
    {
        if (global_platform.key_pressed[KEY_TAB])
            UI_OPEN = !UI_OPEN;
        if (UI_OPEN)
        {
            f32 x_off = 0.05f;
            renderer_push_filled_rect(&rend, v3(0.f + x_off,0.5f, 0.f), v2(0.25f,0.25f),v4(0.2f,0.2f,0.2f,0.9f));
            renderer_push_line(&rend, v3(0.f + x_off,0.5f,0.f), v3(0.25f + x_off,0.5f,0.0), v4(1.f, 0.5f,0.5f,0.9f));
            renderer_push_line(&rend, v3(0.f + x_off,0.5f,0.f), v3(0.f + x_off,1.f,0.0), v4(1.f, 0.5f,0.5f,0.9f));
            renderer_push_line(&rend, v3(0.f + x_off,1.f,0.f), v3(0.25f + x_off,1.f,0.0), v4(1.f, 0.5f,0.5f,0.9f));
            renderer_push_line(&rend, v3(0.25f + x_off,0.5f,0.f), v3(0.25f + x_off,1.f,0.0), v4(1.f, 0.5f,0.5f,0.9f));
            renderer_push_text(&rend, v3(0.05,0.70,0.0), v2(0.02,0.02), "screenshot");
            renderer_push_text(&rend, v3(0.05,0.65,0.0), v2(0.015,0.015), "-format");
            renderer_push_text(&rend, v3(0.05,0.60,0.0), v2(0.015,0.015), "-padding");
        }
    }

    update_animator(&animator);
    renderer_push_model(&rend,&model);
    renderer_push_animated_model(&rend, &animator.model);
    renderer_end_frame(&rend);
}

