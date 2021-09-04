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

typedef struct OBB 
{
    vec3 center;
    vec3 u[3]; //local axes
    vec3 e; //halfwidths along those axes
}OBB;

internal OBB obb_init(vec3 center, f32 *u, vec3 e)
{
    OBB obb;
    obb.center = center;
    obb.e = e;
    for (u32 i = 0; i < 9; ++i)
        ((f32*)obb.u)[i] = u[i];
    return obb;
}

typedef enum ColliderType
{
   BOX = 1,
   SPHERE = 2,
   TRIANGLE = 3,
   ORIENTED_BOUNDED_BOX = 4,
   MAX_COLLIDER_TYPES, 
}ColliderType;
typedef struct SimpleCollider
{
    union 
    {
        AABB box;
        Sphere s;
        OBB obb;
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
    MassData mass_data;
    PhysicsMaterial mat;
    vec3 position;
    vec3 velocity;
    vec3 force;
    f32 gravity_scale;
}SimplePhysicsBody;

internal SimplePhysicsBody simple_physics_body_default(void)
{
    SimplePhysicsBody b;
    b.velocity = v3(0,0,0);
    b.gravity_scale = 1.f;
    b.position = v3(0,0,0);
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

internal void positional_correction(Manifold *m)
{
  f32 percent = 0.6; // usually 20% to 80%
  vec3 correction = vec3_mulf(vec3_divf(m->normal, 1.f), m->penetration / (m->A->mass_data.inv_mass + m->B->mass_data.inv_mass) * percent);
  m->A->position.x -= m->A->mass_data.inv_mass * correction.x;
  m->A->position.y -= m->A->mass_data.inv_mass * correction.y;
  m->A->position.z -= m->A->mass_data.inv_mass * correction.z;
  m->B->position.x += m->B->mass_data.inv_mass * correction.x;
  m->B->position.y += m->B->mass_data.inv_mass * correction.y;
  m->B->position.z += m->B->mass_data.inv_mass * correction.z;
  
}



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
  vec3 a_pos = A->position;
  vec3 b_pos = B->position;
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
                  m->penetration = x_overlap;
              }
              else if (z_overlap < y_overlap && z_overlap < x_overlap)
              {
                  if (n.z > 0)
                      m->normal = v3(0,0,1);
                  else 
                      m->normal = v3(0,0,-1);
                  m->penetration = z_overlap;
              }
              else if (y_overlap < x_overlap && y_overlap < z_overlap)
              {
                  if (n.y > 0)
                      m->normal = v3(0,1,0);
                  else 
                      m->normal = v3(0,-1,0);
                  m->penetration = y_overlap;
              }
              return TRUE;
          }
      }
  }

  return FALSE;
}


//resolves AABB to AABB collsions (updates pos/vel/acc)
internal void resolve_collision_aabb(Manifold *m)
{
    SimplePhysicsBody *A = m->A;
    SimplePhysicsBody *B = m->B;

    // Calculate relative velocity
    vec3 rv = vec3_sub(B->velocity, A->velocity);

    // Calculate relative velocity in terms of the normal direction
    f32 velAlongNormal = vec3_dot( rv, m->normal );

    // Do not resolve if velocities are separating
    if(velAlongNormal > 0.f)return;

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

internal void resolve_collision(Manifold *m)
{
    if (m->A->collider.type == BOX && m->B->collider.type == BOX)
        resolve_collision_aabb(m);
    else
        resolve_collision_aabb(m);
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

typedef struct SimplePhysicsBodyPair
{
    SimplePhysicsBody *A;
    SimplePhysicsBody *B;
}SimplePhysicsBodyPair;

internal void insertion_sort_pairs(SimplePhysicsBodyPair *arr, i32 n)
{
    i32 i, j;
    SimplePhysicsBodyPair key;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;

        while (j >= 0 && (u32)arr[j].A > (u32)key.A) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}
//NOTE: pretty complicated micro optimization, @check
internal u32 cull_dupe_pairs(SimplePhysicsBodyPair *pairs, u32 n)
{ 
    if (n==0)return 0;
    u32 unique_index = 0;
    for (u32 i = 0; i < n; ++i)
    {
        SimplePhysicsBodyPair curr = pairs[i];
        b32 unique = TRUE;
        for (u32 j = 0; j < unique_index; ++j)
            if ((int)pairs[j].A + (int)pairs[j].B == (int)curr.A + (int)curr.B)
                unique = FALSE;
        if (unique)
            pairs[unique_index++] = curr;
    }
    return unique_index;
}

internal void 
obb_get_points(OBB obb, vec3 *arr)
{
    AABB res;
    vec3 center = obb.center;
    vec3 e = obb.e;
    mat4 rotation_matrix = m4d(1.f);
    u32 base_index= 0;

    for (u32 i = 0; i < 3; ++i)
        for (u32 j = 0; j < 3; ++j)
            rotation_matrix.elements[i][j] = ((f32*)obb.u)[base_index++];

    vec3 verts[8] = {
        v3(e.x,e.y,e.z), v3(e.x,e.y,-e.z), v3(e.x,-e.y,e.z), v3(e.x,-e.y,-e.z),
        v3(e.x,e.y,e.z), v3(-e.x,e.y,e.z), v3(e.x,e.y,-e.z), v3(-e.x,e.y,-e.z),
    };

    for (u32 i = 0; i < 8; ++i)
    {
        vec4 local_pos = v4(verts[i].x, verts[i].y, verts[i].z,1.f);
        vec4 rotated_local_pos = mat4_mulv(rotation_matrix, local_pos);
        vec3 global_pos = vec3_add(center, v3(rotated_local_pos.x, rotated_local_pos.y, rotated_local_pos.z));
        verts[i] = global_pos;
    }
}

internal AABB obb_to_aabb(OBB obb)
{
    AABB res;
    vec3 center = obb.center;
    vec3 e = obb.e;
    mat4 rotation_matrix = m4d(1.f);
    u32 base_index= 0;

    for (u32 i = 0; i < 3; ++i)
        for (u32 j = 0; j < 3; ++j)
            rotation_matrix.elements[i][j] = ((f32*)obb.u)[base_index++];

    vec3 verts[30] = {
        v3(e.x,-e.y,-e.z), v3(e.x,e.y,-e.z), v3(e.x,e.y,e.z), v3(e.x,-e.y,e.z),v3(e.x,-e.y,-e.z),
        v3(-e.x,-e.y,-e.z), v3(-e.x,e.y,-e.z), v3(-e.x,e.y,e.z), v3(-e.x,-e.y,e.z),v3(-e.x,-e.y,-e.z),

        v3(-e.x,e.y,-e.z), v3(e.x,e.y,-e.z), v3(e.x,e.y,e.z), v3(-e.x,e.y,e.z),v3(-e.x,e.y,-e.z),
        v3(-e.x,-e.y,-e.z), v3(e.x,-e.y,-e.z), v3(e.x,-e.y,e.z), v3(-e.x,-e.y,e.z),v3(-e.x,-e.y,-e.z),

        v3(-e.x,-e.y,e.z), v3(e.x,-e.y,e.z), v3(e.x,e.y,e.z), v3(-e.x,e.y,e.z),v3(-e.x,-e.y,e.z),
        v3(-e.x,-e.y,-e.z), v3(e.x,-e.y,-e.z), v3(e.x,e.y,-e.z), v3(-e.x,e.y,-e.z),v3(-e.x,-e.y,-e.z),
    };
    for (u32 i = 0; i < 30; ++i)
    {
        vec4 local_pos = v4(verts[i].x, verts[i].y, verts[i].z,1.f);
        vec4 rotated_local_pos = mat4_mulv(rotation_matrix, local_pos);
        vec3 global_pos = vec3_add(center, v3(rotated_local_pos.x, rotated_local_pos.y, rotated_local_pos.z));
        verts[i] = global_pos;
    }

    res.max = v3(-10000.f, -10000.f, -10000.f);
    res.min = v3(10000.f, 10000.f, 10000.f);
    for (u32 i = 0; i < 30; ++i)
    {
        res.max.x = (res.max.x > verts[i].x) ? res.max.x : verts[i].x;
        res.max.y = (res.max.y > verts[i].y) ? res.max.y : verts[i].y;
        res.max.z = (res.max.z > verts[i].z) ? res.max.z : verts[i].z;
        res.min.x = (res.min.x < verts[i].x) ? res.min.x : verts[i].x;
        res.min.y = (res.min.y < verts[i].y) ? res.min.y : verts[i].y;
        res.min.z = (res.min.z < verts[i].z) ? res.min.z : verts[i].z;
    }
    return res;
}
internal OBB aabb_to_obb(AABB box)
{
    OBB obb;
    obb.center = vec3_add(box.min, vec3_divf(vec3_sub(box.max, box.min), 2.f));
    obb.e = vec3_mulf(vec3_sub(box.max, box.min), 1/2.f);
    vec3 arr[3] = {1,0,0,0,1,0,0,0,1};
    for (u32 i = 0; i < 3; ++i)
    {
        obb.u[i] = arr[i];
    }
    return obb;
}

//projects a vector onto an axis
internal f32 project(vec3 v, vec3 axis)
{
    return vec3_dot(v, axis);
}

//                   axis type (a/b/e)| (1-15) | overlap | max distance SAT |  L, test axis | where to put L          
internal b32 track_face_axis(i32 *axis, i32 n, f32 overlap, f32 *min_overlap, vec3 normal, vec3 *axis_normal)
{
    if (overlap > 0.f)
        return TRUE;

    if (overlap > *min_overlap)
    {
        *min_overlap = overlap;
        *axis = n;
        *axis_normal = normal;
    }
    return FALSE;
}

//                   axis type (a/b/e)| (1-15) | overlap | max distance SAT |  L, test axis | where to put L          
internal b32 track_edge_axis(i32 *axis, i32 n, f32 overlap, f32 *min_overlap, vec3 normal, vec3 *axis_normal)
{
    if (overlap > 0.f)
        return TRUE;

    f32 l = 1.f / vec3_length(normal);
    overlap *= l; //we bring the overlap length to unit distance

    if (overlap > *min_overlap)
    {
        *min_overlap = overlap;
        *axis = n;
        *axis_normal = vec3_mulf(normal, l); //also the normal!
    }
    return FALSE;
}


extern char info_log[512]; 

internal b32 
test_obb_obb_manifold(OBB a, OBB b, Manifold *m)
{
    b32 collide = TRUE;
    f32 min_overlap = FLT_MAX;
    vec3 axis_of_min_penetration;

    vec3 nA;//normal in OBB A (from face tests)
    vec3 nB;//normal in OBB B (from face tests)
    vec3 nE;//normal in some edge (from edge tests)

    i32 aMax = -FLT_MAX; //overlap with OBB A (from face tests)
    i32 bMax = -FLT_MAX; //overlap with OBB B (from face tests)
    i32 eMax = -FLT_MAX; //overlap with some edge (from edge tests)

    i32 aAxis = 0; //Axis of overlap for OBB A (from face tests)
    i32 bAxis = 0; //Axis of overlap for OBB B (from face tests)
    i32 eAxis = 0; //Axis of overlap for some edge (from edge tests)



    vec3 t = vec3_sub(b.center, a.center);




    f32 overlap;
    vec3 L; //axis to test collision

    //L = Ax
    L = a.u[0];
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));

    if (track_edge_axis(&aAxis, 1, overlap, &aMax, L, &nA))
        collide = FALSE;

    //L = Ay
    L = a.u[1];
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&aAxis, 2, overlap, &aMax, L, &nA))
        collide = FALSE;



    //L = Az
    L = a.u[2];
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&aAxis, 3, overlap, &aMax, L, &nA))
        collide = FALSE;



    //L = Bx
    L = b.u[0];
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&bAxis, 4, overlap, &bMax, L, &nB))
        collide = FALSE;


    //L = By
    L = b.u[1];
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&bAxis, 5, overlap, &bMax, L, &nB))
        collide = FALSE;


    //L = Bz
    L = b.u[2];
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&bAxis, 6, overlap, &bMax, L, &nB))
        collide = FALSE;

    //L = Ax X Bx 
    L = vec3_cross(a.u[0], b.u[0]);
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&eAxis, 7, overlap, &eMax, L, &nE))
        collide = FALSE;


    //L = Ax X By 
    L = vec3_cross(a.u[0], b.u[1]);
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&eAxis, 8, overlap, &eMax, L, &nE))
        collide = FALSE;

    //L = Ax X Bz 
    L = vec3_cross(a.u[0], b.u[2]);
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&eAxis, 9, overlap, &eMax, L, &nE))
        collide = FALSE;

    //L = Ay X Bx 
    L = vec3_cross(a.u[1], b.u[0]);
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&eAxis, 10, overlap, &eMax, L, &nE))
        collide = FALSE;

    //L = Ay X By 
    L = vec3_cross(a.u[1], b.u[1]);
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&eAxis, 11, overlap, &eMax, L, &nE))
        collide = FALSE;


    //L = Ay X Bz 
    L = vec3_cross(a.u[1], b.u[2]);
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&eAxis, 12, overlap, &eMax, L, &nE))
        collide = FALSE;

   //L = Az X Bx 
    L = vec3_cross(a.u[2], b.u[0]);
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&eAxis, 13, overlap, &eMax, L, &nE))
        collide = FALSE;


   //L = Az X By 
    L = vec3_cross(a.u[2], b.u[1]);
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&eAxis, 14, overlap, &eMax, L, &nE))
        collide = FALSE;


   //L = Az X Bz 
    L = vec3_cross(a.u[2], b.u[2]);
    overlap = fabs(project(t, L)) - fabs(project(vec3_mulf(a.u[0],a.e.x), L)) - fabs(project(vec3_mulf(a.u[1],a.e.y), L)) - fabs(project(vec3_mulf(a.u[2],a.e.z), L)) 
            - fabs(project(vec3_mulf(b.u[0],b.e.x), L)) - fabs(project(vec3_mulf(b.u[1],b.e.y), L)) - fabs(project(vec3_mulf(b.u[2],b.e.z), L));
    if (track_edge_axis(&eAxis, 15, overlap, &eMax, L, &nE))
        collide = FALSE;

    sprintf(info_log, "edge axis: %d", eAxis);




    m->normal = vec3_normalize(t);
    m->penetration = 0.1f;

    return collide;
}


internal b32 
test_obb_obb_manifold_fast(OBB a, OBB b, Manifold *m)
{
    b32 collide = TRUE;
    //do collision stuff
    f32 ra, rb;
    mat3 R, absR;

    vec3 separation_axis = vec3_normalize(vec3_sub(b.center, a.center));
    f32 min_overlap = FLT_MAX;

    /*
    if (overlap < min_overlap)
    {
        min_overlap = overlap;
        separation_axis = v3(0,1,0)
    }
    */
    //Compute rotation matrix expressing b in a's coordinate frame!!
    for (u32 i = 0; i < 3; ++i)
        for (u32 j = 0; j < 3; ++j)
            R.elements[i][j] = vec3_dot(a.u[i], b.u[j]);

    //Compute and transform t into a's coordinate system
    vec3 t = vec3_sub(b.center, a.center);
    t = v3(vec3_dot(t, a.u[0]), vec3_dot(t, a.u[1]), vec3_dot(t, a.u[2]));

    //Compute common subexpressions?????? wtf mr. Ericson???
    for (u32 i = 0; i < 3; ++i)
        for (u32 j = 0; j < 3; ++j)
            absR.elements[i][j] = fabs(R.elements[i][j]) + 0.00001;

    //m->normal = vec3_sub(b.center, a.center);
    m->normal = v3(0,0,0);
    m->penetration = 0.3f;


    f32 overlap;


    //Test axes L = A0, L = A1, L = A2
    for (u32 i = 0; i < 3; ++i)
    {
        ra = a.e.elements[i];
        rb = b.e.x * absR.elements[i][0] + b.e.y * absR.elements[i][1] + b.e.z * absR.elements[i][2];
        overlap = fabs(t.elements[i]) - ra - rb;
        if (overlap > 0)
            collide = FALSE;
    }


    // Test axes L = B0, L = B1, L = B2
    for (u32 i = 0; i < 3; ++i)
    {
        rb = b.e.elements[i];
        ra = a.e.x * absR.elements[0][i] + a.e.y * absR.elements[1][i] + a.e.z * absR.elements[2][i];
        overlap = fabs(t.elements[0] * R.elements[0][i] + t.elements[1] * R.elements[1][i] + t.elements[2] * R.elements[2][i]) - ra - rb;
        if (overlap > 0)
            collide = FALSE;
    }


    //Test L = A0 x B0
    ra = a.e.elements[1] * absR.elements[2][0] + a.e.elements[2] * absR.elements[1][0];
    rb = b.e.elements[1] * absR.elements[0][2] + b.e.elements[2] * absR.elements[0][1];
    overlap = fabs(t.elements[2] * R.elements[1][0] - t.elements[1] * R.elements[2][0]) - ra - rb;
    if (overlap > 0)
        collide = FALSE;
  
    //Test L = A0 x B1
    ra = a.e.elements[1] * absR.elements[2][1] + a.e.elements[2] * absR.elements[1][1];
    rb = b.e.elements[0] * absR.elements[0][2] + b.e.elements[2] * absR.elements[0][0];
    overlap = fabs(t.elements[2] * R.elements[1][1] - t.elements[1] * R.elements[2][1])- ra - rb;
    if (overlap > 0)
        collide = FALSE;
    //Test L = A0 x B2
    ra = a.e.elements[1] * absR.elements[2][2] + a.e.elements[2] * absR.elements[1][2];
    rb = b.e.elements[0] * absR.elements[0][1] + b.e.elements[1] * absR.elements[0][0];
    overlap = fabs(t.elements[2] * R.elements[1][2] - t.elements[1] * R.elements[2][2])- ra - rb;
    if (overlap > 0)
        collide = FALSE;

    //Test L = A1 x B0
    ra = a.e.elements[0] * absR.elements[2][0] + a.e.elements[2] * absR.elements[0][0];
    rb = b.e.elements[1] * absR.elements[1][2] + b.e.elements[2] * absR.elements[1][1];
    overlap =fabs(t.elements[0] * R.elements[2][0] - t.elements[2] * R.elements[0][0]) - ra - rb;
    if (overlap > 0)
        collide = FALSE;
    //Test L = A1 x B1
    ra = a.e.elements[0] * absR.elements[2][1] + a.e.elements[2] * absR.elements[0][1];
    rb = b.e.elements[0] * absR.elements[1][2] + b.e.elements[2] * absR.elements[1][0];
    overlap =fabs(t.elements[0] * R.elements[2][1] - t.elements[2] * R.elements[0][1]) - ra - rb;
    if (overlap > 0)
        collide = FALSE;
    //Test L = A1 x B2
    ra = a.e.elements[0] * absR.elements[2][2] + a.e.elements[2] * absR.elements[0][2];
    rb = b.e.elements[0] * absR.elements[1][1] + b.e.elements[1] * absR.elements[1][0];
    overlap =fabs(t.elements[0] * R.elements[2][2] - t.elements[2] * R.elements[0][2]) - ra - rb;
    if (overlap > 0)
        collide = FALSE;
    //Test L = A2 x B0
    ra = a.e.elements[0] * absR.elements[1][0] + a.e.elements[1] * absR.elements[0][0];
    rb = b.e.elements[1] * absR.elements[2][2] + b.e.elements[2] * absR.elements[2][1];
    overlap =fabs(t.elements[1] * R.elements[0][0] - t.elements[0] * R.elements[1][0]) - ra - rb;
    if (overlap > 0)
        collide = FALSE;
    //Test L = A2 x B1
    ra = a.e.elements[0] * absR.elements[1][1] + a.e.elements[1] * absR.elements[0][1];
    rb = b.e.elements[0] * absR.elements[2][2] + b.e.elements[2] * absR.elements[2][0];
    overlap =fabs(t.elements[1] * R.elements[0][1] - t.elements[0] * R.elements[1][1]) - ra - rb;
    if (overlap > 0)
        collide = FALSE;
    //Test L = A2 x B2
    ra = a.e.elements[0] * absR.elements[1][2] + a.e.elements[1] * absR.elements[0][2];
    rb = b.e.elements[0] * absR.elements[2][1] + b.e.elements[1] * absR.elements[2][0];
    overlap =fabs(t.elements[1] * R.elements[0][2] - t.elements[0] * R.elements[1][2]) - ra - rb;
    if (overlap > 0)
        collide = FALSE;

    m->normal = separation_axis;

    //if (collide) exit(23);
    return collide;
}


internal b32 test_collision_manifold(Manifold *m)
{
    SimplePhysicsBody *A = m->A;
    SimplePhysicsBody *B = m->B;
 
    if (A->collider.type == BOX)
    {
        if (B->collider.type == BOX)
            return test_aabb_aabb_manifold(m);
        else if (B->collider.type == ORIENTED_BOUNDED_BOX)
        {
            OBB obb2 = B->collider.obb;
            OBB obb1 = aabb_to_obb(A->collider.box);
            if (test_obb_obb_manifold(obb1, obb2, m))
                return TRUE;
        }
    }
    else if (A->collider.type == ORIENTED_BOUNDED_BOX)
    {
        if (B->collider.type == BOX)
        {
            OBB obb1 = A->collider.obb;
            OBB obb2 = aabb_to_obb(B->collider.box);
            if (test_obb_obb_manifold(obb1, obb2, m))
                return TRUE;
          }
        else if (B->collider.type == ORIENTED_BOUNDED_BOX)
        {
            if (test_obb_obb_manifold(A->collider.obb, B->collider.obb, m))
               return TRUE;
        }
    }

    return FALSE;
}

internal b32 test_broad_collision_manifold(Manifold *m)
{
    SimplePhysicsBody *A = m->A;
    SimplePhysicsBody *B = m->B;
 
    if (A->collider.type == BOX)
    {
        if (B->collider.type == BOX)
            return test_aabb_aabb_manifold(m);
        else if (B->collider.type == ORIENTED_BOUNDED_BOX)
        {
            OBB obb = B->collider.obb;
            AABB b2 = obb_to_aabb(B->collider.obb);
            if (test_aabb_aabb(b2, A->collider.box))
                return TRUE;
        }
    }
    else if (A->collider.type == ORIENTED_BOUNDED_BOX)
    {
        if (B->collider.type == BOX)
        {
            if (test_aabb_aabb(B->collider.box, obb_to_aabb(A->collider.obb)))
                return TRUE;
        }
        else if (B->collider.type == ORIENTED_BOUNDED_BOX)
        {
            if (test_aabb_aabb(obb_to_aabb(A->collider.obb), obb_to_aabb(B->collider.obb)))
                return TRUE;
        }
    }

    return FALSE;
}

#endif
