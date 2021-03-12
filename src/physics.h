#ifndef PHYSICS_H
#define PHYSICS_H
#include "tools.h" //for math

typedef struct AABB
{
    vec3 min;
    vec3 max;
}AABB;

internal b32 
test_aabb_aabb(AABB a, AABB b)
{
    if (a.max.x < b.min.x || a.min.x > b.max.x) return FALSE;
    if (a.max.y < b.min.y || a.min.y > b.max.y) return FALSE;
    if (a.max.z < b.min.z || a.min.z > b.max.z) return FALSE;
    sprintf(error_log, "collision");
    return TRUE;
}
internal AABB aabb_init(vec3 min, vec3 max)
{
    return (AABB){min, max};
}

typedef struct Sphere
{
    vec3 c;
    f32 r;
}Sphere;

internal b32 test_sphere_sphere(Sphere a, Sphere b)
{
    vec3 d = vec3_sub(a.c,b.c);
    f32 dist2 = vec3_dot(d, d);
    f32 rad_sum = a.r + b.r;
    return (dist2 <= rad_sum * rad_sum);
}

internal Sphere sphere_init(vec3 pos, f32 rad)
{
    return (Sphere){pos, rad};
}


typedef enum ColliderType
{
   BOX = 1,
   SPHERE = 2,
   TRIANGLE = 3,
   MAX_COLLIDER_TYPES, 
}ColliderType;
typedef struct SimpleCollider
{
    union 
    {
        AABB box;
        Sphere s;
    };
    ColliderType type;
}SimpleCollider;

internal SimpleCollider simple_collider_default(void)
{
    SimpleCollider sc;
    sc.box = aabb_init(v3(0,0,0), v3(1,1,1));
    sc.type = BOX;
    return sc;
}

internal SimpleCollider simple_collider_init(vec3 position, vec3 scale)
{
    SimpleCollider sc;
    vec3 min = vec3_sub(position, scale); 
    vec3 max = vec3_add(position, scale); 
    sc.box = aabb_init(min, max);
    sc.type = BOX;
    return sc;
}

internal i32 test_collision(SimpleCollider *c1, SimpleCollider *c2)
{
    if (c1->type == BOX && c2->type == BOX)
        return test_aabb_aabb(c1->box, c2->box);

}

typedef struct MassData
{
    f32 mass;
    f32 inv_mass;
}MassData;
internal MassData mass_data_init(f32 mass)
{
    MassData data;
    data.mass = mass;
    data. inv_mass = 1.f / mass;
    return data;
}
typedef struct PhysicsMaterial
{
    f32 density;
    f32 restitution;
}PhysicsMaterial;

internal PhysicsMaterial physics_material_init(f32 d, f32 r)
{
    PhysicsMaterial mat;
    mat.density = d;
    mat.restitution = r;
    return mat;
}
typedef struct SimplePhysicsBody
{
    SimpleCollider *collider;
    mat4 transform;
    MassData mass_data;
    PhysicsMaterial mat;
    vec3 velocity;
    vec3 force;
}SimplePhysicsBody;

typedef struct Ray
{
    vec3 o;
    f32 t;
    vec3 d;
}Ray;

internal Ray ray_init(vec3 o, f32 t, vec3 d)
{
    return (Ray){o,t,d};
}


//this test doesnt find _the_ solution, just if there is one!
internal i32 
intersect_ray_sphere_simple(Ray ray, Sphere s)
{
    vec3 m = vec3_sub(ray.o, s.c);
    f32 b = vec3_dot(m,ray.d);
    f32 c = vec3_dot(m,m) - (s.r * s.r);
    if (c > 0.f && b > 0.f)return 0;
    f32 disc = b*b - c;
    if (disc < 0.f)return 0;
    return 1;
}

//@TODO Make this fast.. so many variable declarations.....
internal vec3
get_ray_dir(vec2 screen_coords,f32 ww, f32 wh, mat4 view, mat4 proj)
{
    //SCREEN -> 3D NDC
    f32 x = (2.f * screen_coords.x) / (f32)ww - 1.f;
    f32 y = 1.f - (2.f * screen_coords.y) / (f32)wh;
    f32 z = 1.f;
    vec3 ray_ndc = v3(x,y,z);
    //NDC -> HOMOGENEOUS CLIP
    vec4 ray_clip = v4(ray_ndc.x, ray_ndc.y,-1.f,0.f);
    //CLIP -> EYE
    vec4 ray_eye = mat4_mulv(mat4_inv(proj), ray_clip); 
    ray_eye = v4(ray_eye.x, ray_eye.y, -1.f,0.f);
    //EYE -> WORLD
    vec4 ray_world = mat4_mulv(mat4_inv(view), ray_eye);
    vec3 dir = vec3_normalize(v3(ray_world.x,ray_world.y,ray_world.z));
} 


#endif
