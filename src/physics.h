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
    //sprintf(error_log, "collision");
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
    if (equalf(mass, 0.f,0.001))
        data.inv_mass = 0.f;
    else
        data.inv_mass = 1.f / mass;
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
    SimpleCollider collider; //TODO: this better be a pointer
    mat4 transform;
    MassData mass_data;
    PhysicsMaterial mat;
    vec3 velocity;
    vec3 force;
    f32 gravity_scale;
}SimplePhysicsBody;

internal SimplePhysicsBody simple_physics_body_default(void)
{
    SimplePhysicsBody b;
    b.transform = m4d(1.f); //from this we use only the translation??? i think so 
    b.velocity = v3(0,0,0);
    b.gravity_scale = 1.f;
    b.force = v3(0,0,0);
    b.mass_data = mass_data_init(1.f);
    b.mat = physics_material_init(0.5f, 0.7f); //@check
    b.collider = simple_collider_default();
    return b;
}

typedef struct Manifold
{
    SimplePhysicsBody *A;
    SimplePhysicsBody *B;
    f32 penetration;
    vec3 normal;
}Manifold;

internal b32 test_aabb_aabb_manifold(Manifold *m)
{
  // Setup a couple pointers to each object
  SimplePhysicsBody *A = m->A;
  SimplePhysicsBody *B = m->B;
  //default collision values
  m->normal = v3(0,1,0);
  m->penetration = 0.1;

  if (A == B)return;
  
  // Vector from A to B
  vec3 a_pos = v3(A->transform.elements[3][0], A->transform.elements[3][1], A->transform.elements[3][2]);
  vec3 b_pos = v3(B->transform.elements[3][0], B->transform.elements[3][1], B->transform.elements[3][2]);
  vec3 n = vec3_sub(b_pos,a_pos);

  AABB a = A->collider.box;
  AABB b = B->collider.box;

  //calc half extents on x-axis
  f32 a_extent = (a.max.x - a.min.x) / 2.f;
  f32 b_extent = (b.max.x - b.min.x) / 2.f;

  //calc verlap on x-axis
  f32 x_overlap = a_extent + b_extent - fabs(n.x);

  if (x_overlap > 0.f)
  {
      //half intersections on y-axis
      f32 a_extent = (a.max.y - a.min.y) / 2.f;
      f32 b_extent = (b.max.y - b.min.y) / 2.f;

      f32 y_overlap = a_extent + b_extent - fabs(n.y);
      if (y_overlap > 0.f)
      {
          //half intersections on z-axis
          f32 a_extent = (a.max.z - a.min.z) / 2.f;
          f32 b_extent = (b.max.z - b.min.z) / 2.f;

          f32 z_overlap = a_extent + b_extent - fabs(n.z);
      
          if (z_overlap > 0.f)
          {
              if (x_overlap < y_overlap && x_overlap < z_overlap)
              {
                  if (n.x > 0)
                      m->normal = v3(1,0,0);
                  else 
                      m->normal = v3(-1,0,0);
                  //m->penetration = x_overlap;
              }
              else if (z_overlap < y_overlap && z_overlap < x_overlap)
              {
                  if (n.z > 0)
                      m->normal = v3(0,0,1);
                  else 
                      m->normal = v3(0,0,-1);
                  //m->penetration = z_overlap;
              }
              else if (y_overlap < x_overlap && y_overlap < z_overlap)
              {
                  if (n.y > 0)
                      m->normal = v3(0,1,0);
                  else 
                      m->normal = v3(0,-1,0);
                  //m->penetration = y_overlap;
              }
              //n.z = 0;
              //m->normal = vec3_normalize(n); 
              return TRUE;
          }
      }
  }

  return FALSE;
}
internal void resolve_collision(Manifold *m)
{
  SimplePhysicsBody *A = m->A;
  SimplePhysicsBody *B = m->B;
 
  // Calculate relative velocity
  vec3 rv = vec3_sub(B->velocity, A->velocity);
 
  // Calculate relative velocity in terms of the normal direction
  f32 velAlongNormal = vec3_dot( rv, m->normal );
 
  // Do not resolve if velocities are separating
  if(velAlongNormal > 0.f)
    return;
 
  // Calculate restitution
  f32 e = minimum( A->mat.restitution, B->mat.restitution);
 
  // Calculate impulse scalar
  f32 j = -(1.f + e) * velAlongNormal;
  j /= A->mass_data.inv_mass + B->mass_data.inv_mass;
 
  // Apply impulse
  vec3 impulse = vec3_mulf(m->normal,j);
  A->velocity = vec3_sub(A->velocity, vec3_mulf(impulse,A->mass_data.inv_mass));
  B->velocity = vec3_add(B->velocity,vec3_mulf(impulse,B->mass_data.inv_mass));
}

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

internal void positional_correction(Manifold *m)
{
  f32 percent = 0.2f; // usually 20% to 80%
  vec3 correction = vec3_mulf(m->normal, m->penetration / (m->A->mass_data.inv_mass + m->B->mass_data.inv_mass) * percent);
  m->A->transform.elements[3][0] -= m->A->mass_data.inv_mass * correction.x;
  m->A->transform.elements[3][1] -= m->A->mass_data.inv_mass * correction.y;
  m->A->transform.elements[3][2] -= m->A->mass_data.inv_mass * correction.z;
  m->B->transform.elements[3][0] += m->B->mass_data.inv_mass * correction.x;
  m->B->transform.elements[3][1] += m->B->mass_data.inv_mass * correction.y;
  m->B->transform.elements[3][2] += m->B->mass_data.inv_mass * correction.z;
  
}

#endif
