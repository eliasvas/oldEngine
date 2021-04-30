#ifndef ENTITY_H
#define ENTITY_H
#include "tools.h"
#include "phys.h"
#include "platform.h"


#define MAX_ENTITY 1024
#define MAX_COMPONENTS 1024
typedef u32 entity_id;
typedef u32 Entity;
#define INVALID_ENTITY 0

typedef IntHashMap EntityHashMap;
typedef struct PositionComponent
{
   vec3 position; 
}PositionComponent;

typedef struct PositionManager
{
    PositionComponent positions[MAX_COMPONENTS];
    Entity entities[MAX_ENTITY];
    u32 next_index;
    EntityHashMap table;
}PositionManager;

//creates an empty component of type position TODO generalize?
internal PositionComponent*
entity_add_pos(PositionManager *manager, Entity entity)
{
  // INVALID_ENTITY is not allowed!
  assert(entity != INVALID_ENTITY);

  // Only one of this component type per entity is allowed!
  //assert(lookup_hashmap(&manager->table, entity) == -1);

  // Update the entity lookup table:
  hashmap_insert(&manager->table, entity, manager->next_index);

  // New components are always pushed to the end:
  manager->positions[manager->next_index] = (PositionComponent){0}; 

  // Also push corresponding entity:
  manager->entities[manager->next_index] = entity;

  return &manager->positions[manager->next_index++];
}

//removes an entity and its component from the manager's data TODO generalize?
internal void 
entity_remove_pos(PositionManager* manager, Entity entity)
{
  u32 index = hashmap_lookup(&manager->table, entity);
  if (index != -1)
  {
    // Directly index into components and entities array:
    Entity entity = manager->entities[index];

    if (index < manager->next_index)
    {
      // Swap out the dead element with the last one:
      manager->positions[index] = manager->positions[manager->next_index-1];// try to use move
      manager->entities[index] = manager->entities[manager->next_index-1];

      //NOTE: this is not ver performant..
      hashmap_remove(&manager->table,entity);
      hashmap_remove(&manager->table,manager->entities[index]);
      hashmap_insert(&manager->table,manager->entities[index], index); 
    }

    //shrink the container 
    manager->next_index--;
  }
}

internal PositionComponent* 
entity_get_pos(PositionManager *manager, Entity entity)
{
    i32 index = hashmap_lookup(&manager->table, entity);
    if (index != -1)
    {
        return &manager->positions[index];
    }
    return NULL;
}
internal void
position_manager_init(PositionManager *manager)
{
    manager->table = hashmap_create(20);
    manager->next_index = 0;
}



typedef struct Model ModelComponent;


typedef struct ModelManager
{
    ModelComponent models[MAX_COMPONENTS];
    Entity entities[MAX_ENTITY];
    u32 next_index;
    EntityHashMap table;
}ModelManager;

internal ModelComponent*
entity_add_model(ModelManager *manager, Entity entity)
{
  assert(entity != INVALID_ENTITY);
  hashmap_insert(&manager->table, entity, manager->next_index);
  manager->models[manager->next_index] = (ModelComponent){0}; 
  manager->entities[manager->next_index] = entity;
  return &manager->models[manager->next_index++];
}

internal void 
entity_remove_model(ModelManager* manager, Entity entity)
{
  u32 index = hashmap_lookup(&manager->table, entity);
  if (index != -1)
  {
    Entity entity = manager->entities[index];
    if (index < manager->next_index)
    {
      manager->models[index] = manager->models[manager->next_index-1];// try to use move
      manager->entities[index] = manager->entities[manager->next_index-1];
      hashmap_remove(&manager->table,entity);
      hashmap_remove(&manager->table,manager->entities[index]);
      hashmap_insert(&manager->table,manager->entities[index], index); 
    }
    manager->next_index--;
  }
}
internal void
model_manager_init(ModelManager *manager)
{
    manager->table = hashmap_create(20);
    manager->next_index = 0;
}

internal void 
model_manager_reset(ModelManager *manager)
{
    manager->next_index = 0;
    hashmap_reset(&manager->table);
}


typedef SimplePhysicsBody SimplePhysicsBodyComponent;
typedef struct SimulationWorld
{
    SimplePhysicsBodyComponent bodies[MAX_COMPONENTS];
    Entity entities[MAX_ENTITY];
    u32 next_index;
    EntityHashMap table;



    SimplePhysicsBodyPair pairs[1000];
    u32 pairs_count;

    b32 gravity_active;
    b32 damping_active;
}SimulationWorld;

internal SimplePhysicsBodyComponent*
entity_add_rigidbody(SimulationWorld *manager, Entity entity)
{
  assert(entity != INVALID_ENTITY);
  hashmap_insert(&manager->table, entity, manager->next_index);
  manager->bodies[manager->next_index] = (SimplePhysicsBodyComponent){0}; 
  manager->entities[manager->next_index] = entity;
  return &manager->bodies[manager->next_index++];
}

internal void 
entity_remove_rigidbody(SimulationWorld* manager, Entity entity)
{
  u32 index = hashmap_lookup(&manager->table, entity);
  if (index != -1)
  {
    Entity entity = manager->entities[index];
    if (index < manager->next_index)
    {
      manager->bodies[index] = manager->bodies[manager->next_index-1];// try to use move
      manager->entities[index] = manager->entities[manager->next_index-1];
      hashmap_remove(&manager->table,entity);
      hashmap_remove(&manager->table,manager->entities[index]);
      hashmap_insert(&manager->table,manager->entities[index], index); 
    }
    manager->next_index--;
  }
}


internal SimplePhysicsBodyComponent* 
entity_get_rigidbody(SimulationWorld *manager, Entity entity)
{
    i32 index = hashmap_lookup(&manager->table, entity);
    if (index != -1)
    {
        return &manager->bodies[index];
    }
    return NULL;
}


internal void
simworld_init(SimulationWorld *manager)
{
    manager->table = hashmap_create(20);
    manager->next_index = FALSE;
    manager->damping_active = TRUE;
    manager->gravity_active = TRUE;
}
typedef struct EntityManager
{
    Entity next_entity;
    PositionManager position_manager;
    ModelManager model_manager;
    SimulationWorld simworld;
}EntityManager;


internal void collision_table_cleanup(SimulationWorld *manager)
{
    manager->pairs_count = 0;
}
internal void resolve_collisions(SimulationWorld *manager)
{
    Manifold m;
    for (u32 i = 0; i < manager->pairs_count; ++i)
    {
        m.A = manager->pairs[i].A;
        m.B = manager->pairs[i].B;
        //positional_correction(&m);
        if (test_aabb_aabb_manifold(&m))
            resolve_collision(&m);

        manager->bodies[i].velocity= vec3_divf(manager->bodies[i].velocity, 1.005f);
    }

}

internal void 
simworld_reset(SimulationWorld *manager)
{
    manager->next_index = 0;
    hashmap_reset(&manager->table);
    manager->pairs_count = 0;
}

internal void 
entity_manager_init(EntityManager *manager)
{
    manager->next_entity = 0;
    position_manager_init(&manager->position_manager);
    model_manager_init(&manager->model_manager);
    simworld_init(&manager->simworld);
}
internal Entity 
entity_create(EntityManager *manager)
{
   return ++manager->next_entity; 
}
/*
*/

internal i32 last_entity_pressed = -1;
internal void simworld_simulate(SimulationWorld *manager)
{
    collision_table_cleanup(manager);
    Manifold m;
    for (u32 i = 0; i < manager->next_index; ++i)
    {
        SimplePhysicsBody *pb = &manager->bodies[i];
        pb->force = v3(0,0,0);
        mat4 model =pb->transform;
        //pb->transform = mat4_translate(v3(model.elements[3][0], model.elements[3][1], model.elements[3][2]));
        vec3 offset = vec3_sub(pb->collider.box.max, pb->collider.box.min);
        pb->collider.box.min = v3(model.elements[3][0] - offset.x/2, model.elements[3][1] - offset.y/2, model.elements[3][2] - offset.z/2);
        pb->collider.box.max = vec3_add(pb->collider.box.min , offset);
        //this should move from here no??
        if (!equalf(pb->mass_data.inv_mass, 0, 0.001))
            pb->velocity.y -= pb->gravity_scale * global_platform.dt;
    }
    for (u32 i = 0; i < manager->next_index; ++i)
    {
        for (u32 j = 0; j < manager->next_index; ++j)
        {
            SimplePhysicsBodyComponent *pb = &manager->bodies[i];
            if (i == j)continue;
            m.A = &manager->bodies[i];
            m.B = &manager->bodies[j];
            if (test_aabb_aabb_manifold(&m))
                manager->pairs[manager->pairs_count++] = (SimplePhysicsBodyPair){m.A, m.B};
        }
        
    }
    //TODO here do the test intersections!!
    insertion_sort_pairs(manager->pairs, manager->pairs_count);
    manager->pairs_count = cull_dupe_pairs(manager->pairs, manager->pairs_count);
    resolve_collisions(manager);
}
internal void mouse_pick_phys(SimulationWorld *manager, Renderer *rend)
{

    Ray r =  (Ray){rend->cam.pos, 1, v3(0,0,0)};
    r.d = get_ray_dir(v2(global_platform.mouse_x, global_platform.mouse_y),global_platform.window_width, global_platform.window_height, rend->view, rend->proj);//(Ray){v3(0,0,0), 1, v3(0,0,-1)};
    for (u32 i = 0; i < manager->next_index; ++i)
    {
        SimplePhysicsBody *pb = &manager->bodies[i];
        mat4 model = manager->bodies[i].transform;
        vec3 pos = v3(model.elements[3][0], model.elements[3][1], model.elements[3][2]);
        i32 collision = intersect_ray_sphere_simple(r, (Sphere){pos, 0.5});
        if (collision && global_platform.right_mouse_down || last_entity_pressed == i)
        {
            //sprintf(error_log, "good collision, mouse_dt = %f, %f", global_platform.mouse_dt.x, global_platform.mouse_dt.y);
            last_entity_pressed = i;
            vec3 right = vec3_normalize(vec3_cross(rend->cam.front,rend->cam.up));
            vec3 up = vec3_normalize(rend->cam.up);
            pb->velocity.x += vec3_mulf(right, ((-1.f)*(f32)global_platform.mouse_dt.x / global_platform.window_width)).x;
            pb->velocity.y += vec3_mulf(up,((f32)1.f * global_platform.mouse_dt.y / global_platform.window_height)).y;
            //sprintf(error_log, "collision detected!!");
        }
        else if (global_platform.right_mouse_down && last_entity_pressed == i) {
            last_entity_pressed = i;
            //sprintf(error_log, "collision detected!!");
        }
        else if (!global_platform.right_mouse_down)
            last_entity_pressed = -1;
    }


}

internal void 
entity_manager_update(EntityManager *manager, Renderer *rend)
{

    simworld_simulate(&manager->simworld);
    mouse_pick_phys(&manager->simworld, rend);

    for (u32 i = 0; i < manager->model_manager.next_index; ++i)
    {
        SimplePhysicsBody *current = manager->model_manager.models[i].physics_body;
        //current->force = vec3_mulf(current->force, global_platform.dt);
        //current->velocity = vec3_add(current->velocity, vec3_mulf(current->force,global_platform.dt));
        vec3 transform = current->velocity; //*dt???
        current->transform.elements[3][0] += transform.x;
        current->transform.elements[3][1] += transform.y;
        current->transform.elements[3][2] += transform.z;
        //update the _model's_ position
        manager->model_manager.models[i].model.elements[3][0] = current->transform.elements[3][0];
        manager->model_manager.models[i].model.elements[3][1] = current->transform.elements[3][1];
        manager->model_manager.models[i].model.elements[3][2] = current->transform.elements[3][2];

    }
}
internal void 
entity_manager_render(EntityManager *manager, Renderer *rend)
{
    if (last_entity_pressed >= 0)
    {
        mat4 model = manager->model_manager.models[last_entity_pressed].model;
        vec3 pos = v3(model.elements[3][0], model.elements[3][1], model.elements[3][2]);
        renderer_push_compass(rend, pos);
    }
    for (u32 i = 0; i < manager->model_manager.next_index; ++i)
    {
        renderer_push_model(rend, manager->model_manager.models[i]);
        renderer_push_cube_wireframe(rend, manager->model_manager.models[i].physics_body->collider.box.min,manager->model_manager.models[i].physics_body->collider.box.max);
    }
}

internal void 
entity_manager_reset(EntityManager *manager)
{
    simworld_reset(&manager->simworld);
    model_manager_reset(&manager->model_manager);
    manager->next_entity = 0;
}
#include "stdio.h"
void scene_init(char *filepath, EntityManager * manager)
{
    //cleanup already existing models (each model has a MeshInfo pointer which need cleaning)
    for (u32 i = 0; i < manager->model_manager.next_index; ++i)
    {
        Model *m = &manager->model_manager.models[i];
        FREE(m->meshes);
    }

    entity_manager_reset(manager);
    FILE *file = fopen(filepath, "r");
    if (!file)return;
    Model *m;
    char str[256];
    vec3 pos;
    vec3 scale;
    f32 angle;
    vec3 axis;
    while(TRUE)
    {
        i32 res = fscanf(file, "%s", str);
        if (res == EOF || res == 0){fclose(file);return;}
        fscanf(file,"%f %f %f", &pos.x, &pos.y, &pos.z);
        fscanf(file,"%f %f %f", &scale.x, &scale.y, &scale.z);
        if (strcmp("CUBE", str) == 0)
        {
            m = entity_add_model(&manager->model_manager,entity_create(manager));
            model_init_cube(m);
            m->model = mat4_mul(mat4_translate(pos), mat4_mul(mat4_rotate(0.0, v3(1,1,0)), mat4_scale(scale)));
           
            m->physics_body = entity_add_rigidbody(&manager->simworld,entity_create(manager));
            *(m->physics_body) = simple_physics_body_default();
            m->physics_body->collider = simple_collider_init(pos,scale);
            m->physics_body->gravity_scale = 1.f;

            m->physics_body->transform = mat4_mul(mat4_translate(pos), mat4_mul(mat4_rotate(0.0, v3(1,1,0)), mat4_scale(scale)));
            if (scale.x * scale.y * scale.z > 5.f)
            {
                m->physics_body->mass_data = mass_data_init(0.f);
            }
            else
            {
                m->physics_body->mass_data = mass_data_init(1.f);
                //m->physics_body->mat.restitution = 0.1f;
            }
        }
        else if (strcmp("SPHERE", str) == 0)
        {
            m = entity_add_model(&manager->model_manager,entity_create(manager));
            model_init_sphere(m, 1, 20, 20);
            m->model = mat4_mul(mat4_translate(pos), mat4_scale(scale));

           
            m->physics_body = entity_add_rigidbody(&manager->simworld,entity_create(manager));
            *(m->physics_body) = simple_physics_body_default();
            m->physics_body->collider = simple_collider_default();
            m->physics_body->gravity_scale = 0.f;
            m->physics_body->transform= mat4_mul(mat4_translate(pos), mat4_scale(scale));
            //sprintf(error_log, "sphere done");
        }
        else
        {
            fscanf(file,"%f", &angle);
            fscanf(file,"%f %f %f", &axis.x, &axis.y, &axis.z);
            m = entity_add_model(&manager->model_manager,entity_create(manager));
            (*m) = model_info_init(str);
            (*m).model = mat4_mul(mat4_translate(pos),mat4_mul(mat4_rotate(angle, axis), mat4_scale(scale)));
            //TODO put a physics body
            //m->physics_body = simple_physics_body_default();
            //m->physics_body.collider = simple_collider_default();
        }

    }

    fclose(file);
}
#endif
