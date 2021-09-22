#include "platform.h"
#include "tools.h"
#include "objloader.h"
#include "thread.h"
#include "phys.h"
#include "camera.h"
#include "model.h"
#include "quad.h" 
#include "fbo.h"
#include "renderer.h"
#include "collada_parser.h"
#include "animation.h"
#include "entity.h"
#include "dui.h"
#include "net.h"
#include "particle_system.h"
#include "sound.h"

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


global ParticleEmitter pe;
/*
 Engine TODO:
    -Physics Engine (Stabilize current version..)
    -Cascaded Shadow Maps!!!
    -Clean Shader grammar?!?!?!
    -Fix Bloom effects
    -Light Attenuation and radius stuff
    -IMGUI tweaks (add size member configure events and stuff) 
    -Normal visualization is prb wrong, FIX
    -2D sprites (projected in 3d space) w/animations
    -Make the engine a LIB file
    -Collada Parser Overhaul
    -SSAO with data from depth pass
    -GI????????
*/
internal void 
init(void)
{
    entity_manager_init(&entity_manager);
    scene_init("../assets/base.txt", &entity_manager);

    model_init_cube(&debug_cube, m4d(1.f));
    renderer_init(&rend);
    model_init_cube(&light_cube, m4d(1.f));
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
    particle_emitter_init(&pe, v3(0,1,20));
}



internal void 
update(void)
{
    entity_manager_update(&entity_manager, &rend);
    animation_controller_update_works(&ac);
    renderer_begin_frame(&rend);
    rend.cam.can_rotate = !UI_OPEN;
    if (global_platform.key_pressed[KEY_0])
        scene_init("../assets/empty_scene.txt", &entity_manager);
    if (global_platform.key_pressed[KEY_1])
        scene_init("../assets/base.txt", &entity_manager);
    else if (global_platform.key_pressed[KEY_2])
        scene_init("../assets/csm_test.txt", &entity_manager);
    else if (global_platform.key_pressed[KEY_3])
        scene_init("../assets/pbr_test.txt", &entity_manager);
    else if (global_platform.key_pressed[KEY_4])
        scene_init("../assets/oit_test.txt", &entity_manager);

    //renderer_push_billboard(&rend, v3(0,10,0), v4(1,0,1,1));
    particle_emitter_simulate(&pe);

    if (global_platform.key_pressed[KEY_U])
    {
        animation_controller_play_anim(&ac,1);
    }

    if (global_platform.key_pressed[KEY_I])
    {
        animation_controller_play_anim(&ac,0);
    }
 }

internal void 
render(void)
{
    sphere.model = mat4_mul(mat4_translate(v3(0,0,0)), mat4_scale(v3(0.5,0.5,0.5)));
    //renderer_push_model(&rend, &sphere);
    model.model = mat4_mul(mat4_translate(v3(0,-2,0)), mat4_scale(v3(40,0.1,40)));
    //renderer_push_model(&rend, &model);
    
    /*
    PointLight pl = point_light_init(v3(3*sin(global_platform.current_time),0,3*cos(global_platform.current_time)),v3(0.6,0.5,0.7),v3(9,8,8),v3(9,8,8));
    for (u32 i = 0;i< 200; ++i)
    {

        RendererBillboard b = rend.billboard_instance_data[i];
        pl.diffuse = v3(b.color.x, b.color.y, b.color.z);
        pl.specular= v3(b.color.x, b.color.y, b.color.z);
        pl.position = b.center;
        renderer_push_point_light(&rend,pl);
        //renderer_push_point(&rend, pl.position, v4(1,1,1,1));
    }
    */
   
    /*
    //NOTE: obb rendering test
    {

        mat4 rotation_matrix = mat4_rotate(360.f * sin(global_platform.current_time), v3(0.22 * sin(global_platform.current_time * 1.23),1,0.2 * sin(global_platform.current_time * 3.2f)));
        mat3 axes = mat4_extract_rotation(rotation_matrix);

        vec3 center = v3(2,5,3);
        vec3 hw = v3(0.3f,0.3f,0.3f);
        renderer_push_obb_wireframe(&rend, center, (f32*)axes.elements, hw);
        OBB test_obb = obb_init(center, (f32*)axes.elements, hw);
        AABB bounding_box = obb_to_aabb(test_obb);
        renderer_push_cube_wireframe(&rend, bounding_box.min, bounding_box.max);
    }
    */

    particle_emitter_render(&pe, &rend);
    dui_frame_begin();

    //UI bullshit..
    {
        if (global_platform.key_pressed[KEY_TAB])
            UI_OPEN = !UI_OPEN;
        if (UI_OPEN)
        {
            u32 e = 10;
            dui_draw_rect(200 - e, 200 - e, DUI_DEF_X + 2 * e, 150 +2 * e, v4(0.1,0.1,0.1,0.9));
            char ms[64];
            sprintf(ms, "samples: %i", rend.multisampling_count);

            dui_slider_text(GEN_ID, 200 ,300, 8,ms, &rend.multisampling_count);
            dui_switch_text(GEN_ID, (dui_Rect){200,250,DUI_DEF_X,DUI_DEF_Y}, "SSAO", &rend.renderer_settings.ssao_on);
            if(dui_button_text(GEN_ID, (dui_Rect){200,200,DUI_DEF_X,DUI_DEF_Y}, "blip"))
                play_sound("../assets/blip.wav");



            sprintf(ms, "%.4f ms", global_platform.dt);
            renderer_push_text(&rend, v3(0.82,0.90,0.0), v2(0.015,0.025), ms);
            dui_draw_string(200, 170, info_log);

        }
    }

    entity_manager_render(&entity_manager, &rend);
    //dui_switch(GEN_ID, (dui_Rect){0,0,100,100}, &UI_OPEN);
    renderer_push_animated_model(&rend, &ac.model);
    dui_frame_end();
    renderer_end_frame(&rend);
}

