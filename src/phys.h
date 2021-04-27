#ifndef PHYS_H
#define PHYS_H
#include "tools.h"

extern Renderer rend;
typedef struct BoxCollider
{
    vec3 min;
    vec3 max;
}BoxCollider;
typedef struct RigidBody
{
    f32 width, height;
    f32 COR;
    f32 inv_mass;
}RigidBody;
#define MAX_RIGID_BODIES 1000
typedef struct SimulationWorld
{
    RigidBody bodies[MAX_RIGID_BODIES];
    u32 rb_count;


    vec2 collision_normal;
    i32 colliding_body_index;
    i32 colliding_corner_index;

    f32 world_width, world_height;

    b32 damping_active;
    b32 gravity_active;
    b32 world_spring_active;
    b32 body_spring_active;
}SimulationWorld;
internal SimulationWorld sw;

internal RigidBody rb_init(f32 density, f32 w, f32 h, f32 COR)
{
    RigidBody res;

    f32 mass = density * w * h;
    res.inv_mass = 1.f / mass;

    res.width = w;
    res.height = h;
    res.COR = COR;

    return res;
}

internal void sw_init(void)
{
   sw.rb_count = 0; 
   sw.world_width = 1000;
   sw.world_height = 1000;
   sw.damping_active = TRUE;
   sw.gravity_active = TRUE;
}
internal void sw_simulate(f32 dt)
{
    for (u32 i = 0; i < sw.rb_count; ++i)
    {
        //simulate the darn rigid body.
    }
}
internal void sw_render(void)
{
    renderer_push_cube_wireframe(&rend, v3(-4,-4,0), v3(100,100,100));
    for (u32 i = 0; i < sw.rb_count; ++i)
    {
        //render the darn rigid body.
    }
}

#endif
