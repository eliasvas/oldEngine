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
    -[]Physics Engine (Stabilize current version..)
    -[]Cascaded Shadow Maps!!!
    -[]Check Vertex normals for OBJ loading
    -[]Fix Timestepping!!!!
    -[]Light Attenuation and radius stuff
    -[]See how UE4 does their PBR lighting
    -[]IMGUI tweaks (add size member configure events and stuff) 
    -[]Normal visualization is prb wrong, FIX
    -[]2D sprites (projected in 3d space) w/animations
    -[]Make the engine a LIB file
    -[]Collada Parser Overhaul
*/
internal void 
init(void)
{
    entity_manager_init(&entity_manager);
    scene_init("../assets/scene2.txt", &entity_manager);

    model_init_cube(&debug_cube);
    renderer_init(&rend);
    model_init_cube(&light_cube);
    model_init_sphere(&sphere, 2.f, 50,50);
    model = model_info_init("../assets/arena/arena.mtl");

    ac = animation_controller_init(str(&global_platform.frame_storage,"../assets/bender/bender.tga"), str(&global_platform.frame_storage,"../assets/bender/bender.dae"), str(&global_platform.frame_storage,"../assets/bender/bender.dae"));  
    animation_controller_add_anim(&ac,str(&global_platform.frame_storage,"../assets/bender/run.dae"));
    animation_controller_add_anim(&ac,str(&global_platform.frame_storage,"../assets/bender/kick.dae"));
    ac.model.model = mat4_translate(v3(2,-1.8,0));
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
  //animation_controller_update(&ac);
  renderer_begin_frame(&rend);
  rend.cam.can_rotate = !UI_OPEN;
  if (global_platform.key_pressed[KEY_P])
    scene_init("../assets/scene2.txt", &entity_manager);
  else if (global_platform.key_pressed[KEY_O])
    scene_init("../assets/scene.txt", &entity_manager);

  /* These dont serve any purpose right now.. TODO make an ASM (animation state machine) for player at least.
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
    sphere.model = mat4_mul(mat4_translate(v3(0,0,0)), mat4_scale(v3(0.5,0.5,0.5)));
    renderer_push_model(&rend, &sphere);
    model.model = mat4_mul(mat4_translate(v3(0,-2,0)), mat4_scale(v3(40,0.1,40)));
    renderer_push_model(&rend, &model);
     
    PointLight pl = point_light_init(v3(3*sin(global_platform.current_time),0,3*cos(global_platform.current_time)),v3(6,5,7),v3(9,8,8),v3(9,8,8));
    for (u32 i = 0;i< 1; ++i)
    {
        pl.position.y += 0.5; 
        light_cube.model = mat4_translate(pl.position);
        renderer_push_point_light(&rend,pl);
        renderer_push_point(&rend, pl.position, v4(1,1,1,1));
        //renderer_push_model(&rend, &light_cube);
    }
    //sprintf(info_log,"point light count: %i", rend.point_light_count);
    //sprintf(info_log,"camera pos: %f,%f,%f", rend.cam.pos.x, rend.cam.pos.y, rend.cam.pos.z);

    dui_frame_begin();

    dui_draw_string(320, 180, info_log);
    //UI bullshit..
    {
        if (global_platform.key_pressed[KEY_TAB])
            UI_OPEN = !UI_OPEN;
        if (UI_OPEN)
        {
            dui_draw_rect(200, 200, 270, 200, v4(0.1,0.1,0.1,0.9));
            do_slider(GEN_ID, 200 ,300, 4, &rend.multisampling_count);
            do_switch(GEN_ID, (dui_Rect){200,270,20,20}, &rend.renderer_settings.light_cull);
            do_switch(GEN_ID, (dui_Rect){200,240,20,20}, &rend.renderer_settings.debug_mode);
            do_button(GEN_ID, (dui_Rect){260,200,150,30});
            dui_draw_string(260, 370, "options");
            dui_draw_string(190, 330, "multisampling");
            dui_draw_string(215, 275, "light cull");
            dui_draw_string(230, 240, "debug");
            dui_draw_string(280, 210, " PAUSE");
            char ms[64];
            sprintf(ms, "%.4f ms", global_platform.dt);
            renderer_push_text(&rend, v3(0.82,0.90,0.0), v2(0.015,0.025), ms);
            sprintf(ms, "permanent %i/%i", global_platform.permanent_storage.current_offset, global_platform.permanent_storage.memory_size);
            dui_draw_string(380, 310, ms);
            sprintf(ms, "frame %i/%i", global_platform.frame_storage.current_offset, global_platform.frame_storage.memory_size);
            dui_draw_string(380, 290, ms);
        }
    }

    entity_manager_render(&entity_manager, &rend);
    do_switch(GEN_ID, (dui_Rect){0,0,100,100}, &UI_OPEN);
    renderer_push_animated_model(&rend, &ac.model);
    dui_frame_end();
    renderer_end_frame(&rend);
}

