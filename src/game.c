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
global AnimationController ac;
global Coroutine *co;

global b32 UI_OPEN;

global EntityManager entity_manager;


/*
 Engine TODO:
    -[]IMGUI tweaks (add size member configure events and stuff) 
    -[WIP]Animation Overhaul (Add many animations on controller + additive Blenndig)
    -[]Physics Engine (Stabilize current version..)
    -[]OBJ Loader (add -o support and we'll be mostly complete)
*/
internal void 
init(void)
{
    entity_manager_init(&entity_manager);
    scene_init("../assets/scene2.txt", &entity_manager);

    model_init_cube(&debug_cube);
    renderer_init(&rend);
    model_init_cube(&light_cube);
    model_init_sphere(&sphere, 2.f, 20,20);
    model = model_info_init("../assets/sword/sword.mtl");

    ac = animation_controller_init(str(&global_platform.frame_storage,"../assets/bender/bender.tga"), 
        str(&global_platform.frame_storage,"../assets/bender/bender.dae"), str(&global_platform.frame_storage,"../assets/bender/bender.dae"));  
    animation_controller_add_anim(&ac,str(&global_platform.frame_storage,"../assets/bender/run.dae"));
    animation_controller_add_anim(&ac,str(&global_platform.frame_storage,"../assets/bender/kick.dae"));
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
  animation_controller_update(&ac);
  renderer_begin_frame(&rend);
  rend.cam.can_rotate = !UI_OPEN;
  if (global_platform.key_pressed[KEY_P])
    scene_init("../assets/scene2.txt", &entity_manager);
  else if (global_platform.key_pressed[KEY_O])
    scene_init("../assets/scene.txt", &entity_manager);

  //ac controller test
  {
      if (global_platform.key_down[KEY_UP])
          ac.model.model.elements[3][2] += global_platform.dt * 10;
      if (global_platform.key_down[KEY_DOWN])
          ac.model.model.elements[3][2] -= global_platform.dt * 10;

      if (global_platform.key_down[KEY_LEFT])
          ac.model.model.elements[3][0] += global_platform.dt * 10;
      if (global_platform.key_down[KEY_RIGHT])
          ac.model.model.elements[3][0] -= global_platform.dt * 10;
  }
  /* These dont serve any purpose right now.. TODO make am ASM (animation state machine) for player at least.
    if (global_platform.key_pressed[KEY_U])
    {
        ac.animation_time = 0;
        animation_controller_play_anim(&ac,1);
    }
    if (global_platform.key_pressed[KEY_I])
    {
        ac.animation_time = 0;
        animation_controller_play_anim(&ac,0);
    }
    */
}

internal void 
render(void)
{
    renderer_push_point_light(&rend,(PointLight){v3(40*sin(global_platform.current_time),5,40*cos(global_platform.current_time)),
        1.f,0.09f,0.0032f,v3(6,5,7),v3(9,8,8),v3(9,8,8),256.f});

    //renderer_push_model(&rend, &sphere);
    light_cube.model = mat4_translate(v3(40*sin(global_platform.current_time),5,40*cos(global_platform.current_time)));
    model.model = mat4_mul(animation_controller_socket(&ac, 16, mat4_translate(v3(0,0,0.001))),m4d(1.f));
    model.model = mat4_mul(model.model, quat_to_mat4(quat_from_angle(v3(0,0,1), 80)));
    model.model = mat4_mul(model.model, mat4_scale(v3(0.02,0.02,0.02)));
    renderer_push_model(&rend, &model);

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
    renderer_push_animated_model(&rend, &ac.model);
    dui_frame_end();
    renderer_end_frame(&rend);
}

