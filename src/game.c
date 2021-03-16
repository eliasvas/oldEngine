#include "platform.h"
#include "tools.h"
#include "objloader.h"
#include "physics.h"
#include "camera.h"
#include "model.h"
#include "quad.h" 
#include "fbo.h"
#include "renderer.h"
#include "collada_parser.h"
#include "animation.h"
#include "entity.h"
#include "dui.h"
mat4 view,proj;

global Model debug_cube;
global Model light_cube;
global Model sphere;
global Model model;
Renderer rend;
global Animator animator;
global Coroutine *co;

global b32 UI_OPEN;

global EntityManager entity_manager;
internal void 
init(void)
{
    entity_manager_init(&entity_manager);
    scene_init("../assets/scene.txt", &entity_manager);

    model_init_cube(&debug_cube);
    renderer_init(&rend);
    model_init_cube(&light_cube);
    light_cube.meshes[0].material.diff = debug_cube.meshes[0].material.spec;
    model_init_sphere(&sphere, 2.f, 20,20);

    model = model_info_init("../assets/arena/arena.mtl");
    model.model = mat4_scale(v3(0.2f,0.2f,0.2f));
    animator = animator_init(str(&global_platform.frame_storage,"../assets/bender/bender.tga"), 
        str(&global_platform.frame_storage,"../assets/bender/bender.dae"), str(&global_platform.frame_storage,"../assets/bender/bender.dae"));  
    dui_default();
    {
        co = ALLOC(sizeof(Coroutine));
        coroutine_init(co);
    }

}



internal void 
update(void)
{
  entity_manager_update(&entity_manager, &rend);
  renderer_begin_frame(&rend);
  rend.cam.can_rotate = !UI_OPEN;
  if (global_platform.key_pressed[KEY_P])
    scene_init("../assets/scene2.txt", &entity_manager);
  else if (global_platform.key_pressed[KEY_O])
    scene_init("../assets/scene.txt", &entity_manager);
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
    //renderer_push_model(&rend, &light_cube);
    sphere.model = mat4_mul(mat4_translate(v3(0,5,0)),mat4_scale(v3(0.2f,0.2f,0.2f)));
    //renderer_push_model(&rend, &sphere);

    dui_frame_begin();
    //UI bullshit..
    {
        if (global_platform.key_pressed[KEY_TAB])
            UI_OPEN = !UI_OPEN;
        if (UI_OPEN)
        {
            dui_draw_rect(200, 200, 270, 200, v4(0.1,0.1,0.1,0.9));
            do_slider(GEN_ID, 200 ,300, 4, &rend.multisampling_count);
            do_switch(GEN_ID, (dui_Rect){200,270,20,20}, &rend.renderer_settings.motion_blur);
            if (do_switch(GEN_ID, (dui_Rect){200,240,20,20}, &rend.renderer_settings.z_prepass))rend.renderer_settings.z_prepass = 0;
            if (do_switch(GEN_ID, (dui_Rect){220,240,20,20}, &rend.renderer_settings.z_prepass))rend.renderer_settings.z_prepass = 1;
            do_button(GEN_ID, (dui_Rect){260,200,150,30});
            dui_draw_string(260, 370, "options");
            dui_draw_string(190, 330, "multisampling");
            dui_draw_string(215, 275, "antialiasing");
            dui_draw_string(230, 240, "Z-Prepass");
            dui_draw_string(280, 210, " PAUSE");
            char ms[64];
            sprintf(ms, "%.4f ms", global_platform.dt);
            renderer_push_text(&rend, v3(0.82,0.90,0.0), v2(0.015,0.025), ms);
        }
    }
    entity_manager_render(&entity_manager, &rend);
    do_switch(GEN_ID, (dui_Rect){0,0,100,100}, &UI_OPEN);
    dui_frame_end();

    update_animator(&animator);
    renderer_push_animated_model(&rend, &animator.model);
    renderer_end_frame(&rend);
}

