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

//PositionManager is just a dummy manager for me to remember how this whole thing works!
typedef struct PositionManager
{
    PositionComponent positions[MAX_COMPONENTS];
    Entity entities[MAX_ENTITY];
    u32 next_index;
    EntityHashMap table;
}PositionManager;

//entity_add_x creates an empty component of type x 
internal PositionComponent*
entity_add_pos(PositionManager *manager, Entity entity)
{
  // INVALID_ENTITY(0) is not allowed!
  assert(entity != INVALID_ENTITY);

  // Only one of this component type per entity is allowed!
  //assert(lookup_hashmap(&manager->table, entity) == -1);

  // Insert this entity in the hashmap lookup table, {entity, manager->next_index}
  // We need the hashmap so we can do fast lookup, for example if we want to query the position component of some entity!
  hashmap_insert(&manager->table, entity, manager->next_index);

  // New components are always pushed to the end (we make an empty position to return)
  manager->positions[manager->next_index] = (PositionComponent){0}; 

  // We put the entity id as the entity for the current index (manager->next_index)
  manager->entities[manager->next_index] = entity;

  // Finally we return the empty position so that the invoker can initialize
  // AND we increment the next_index for the Manager so that its ready for a new Component initialization!
  return &manager->positions[manager->next_index++];
}

//entity_remove_x removes an entity and its component from the manager's data
internal void 
entity_remove_pos(PositionManager* manager, Entity entity)
{

    // First we do a lookup to find the index where the entities' component is
    // inside the Managers' data array.
    u32 index = hashmap_lookup(&manager->table, entity);
    // If (index == -1) then there is no component for that entity
    // on the manager we are looking to remove, so we do nothing.
    if (index != -1)
    {
        Entity entity = manager->entities[index];

        // If the component was found, we see if (index < manager->next_index), making sure
        // it is a real component and not garbage or something we already have deleted.
        if (index < manager->next_index)
        {
          // Swap out the dead element with the last one (which is a valid component) in its place.
          manager->positions[index] = manager->positions[manager->next_index-1];
          manager->entities[index] = manager->entities[manager->next_index-1];

          // Delete the old entity id from the hashmap so if we try to index/remove it
          // again nothing will be found, then do the same thing for the last element
          // since it moved to the deleted entities place and FINALLY and most importantly
          // we add to the hashmap the pair {entities[index], index}, which is the last component
          // we swapped and its corresponding index in the Managers array!
          hashmap_remove(&manager->table,entity);
          hashmap_remove(&manager->table,manager->entities[index]);
          hashmap_insert(&manager->table,manager->entities[index], index); 
        }

        // At the end we shrink the container, making the last element we swapped simple garbage
        manager->next_index--;
    }
}

internal PositionComponent* 
entity_get_pos(PositionManager *manager, Entity entity)
{
    // We lookup the managers' hashmap to find the component
    // corresponding to Entity entity
    i32 index = hashmap_lookup(&manager->table, entity);

    // If (index != -1), it means we found a component for this entity
    if (index != -1)
    {
        return &manager->positions[index];
    }

    // If (index == -1) no such component was found so we just return NULL
    return NULL;
}
// x_manager_init makes a manager of type x
internal void
position_manager_init(PositionManager *manager)
{
    // First we create a hashmap for the {entity, index} mappings
    manager->table = hashmap_create(20);
    // Then we make the next_index 0, so that we can write new components
    // to the managers' array starting from position 0 (the start of the array :P)
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



    SimplePhysicsBodyPair pairs[1024];
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
        if (test_collision_manifold(&m))
        {
            resolve_collision(&m);
            positional_correction(&m);
        }

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

        //find collider min/max
        vec3 offset = vec3_sub(pb->collider.box.max, pb->collider.box.min);
        if (pb->collider.type == BOX)
        {
            pb->collider.box.min = v3(pb->position.x - offset.x/2, pb->position.y - offset.y/2, pb->position.z - offset.z/2);
            pb->collider.box.max = vec3_add(pb->collider.box.min , offset);
        }
        else if (pb->collider.type == ORIENTED_BOUNDED_BOX)
        {
            pb->collider.obb.center = pb->position;
        }

        //integrate forces!
        pb->velocity = vec3_add(pb->velocity, vec3_mulf(vec3_add(vec3_mulf(v3(0,pb->gravity_scale, 0), -100.f), vec3_mulf(pb->force, pb->mass_data.inv_mass)), global_platform.dt));
        pb->force = v3(0,0,0);
    }
    //broad phase collision detection
    for (u32 i = 0; i < manager->next_index; ++i)
    {
        for (u32 j = i + 1; j < manager->next_index; ++j)
        {
            m.A = &manager->bodies[i];
            m.B = &manager->bodies[j];
            if (test_broad_collision_manifold(&m))
                manager->pairs[manager->pairs_count++] = (SimplePhysicsBodyPair){m.A, m.B};
        }
        
    }
    //sort broad phase pairs and cull dupes
    insertion_sort_pairs(manager->pairs, manager->pairs_count);
    //manager->pairs_count = cull_dupe_pairs(manager->pairs, manager->pairs_count); //TODO(ilias): improve the culling part :((((
    //integrate velocity
    for (u32 i = 0; i < manager->next_index; ++i)
    {
        SimplePhysicsBodyComponent *pb = &manager->bodies[i];
        pb->position = vec3_add(pb->position, vec3_mulf(pb->velocity, global_platform.dt));
    }
    //sprintf(info_log, "number of pairs %i", manager->pairs_count);
    resolve_collisions(manager);
}


internal void mouse_pick_phys(SimulationWorld *manager, Renderer *rend)
{

    Ray r =  (Ray){rend->cam.pos, 1, v3(0,0,0)};
    r.d = get_ray_dir(v2(global_platform.mouse_x, global_platform.mouse_y),global_platform.window_width, global_platform.window_height, rend->view, rend->proj);//(Ray){v3(0,0,0), 1, v3(0,0,-1)};
    for (u32 i = 0; i < manager->next_index; ++i)
    {
        SimplePhysicsBody *pb = &manager->bodies[i];
        vec3 pos = v3(pb->position.x, pb->position.y, pb->position.z);
        i32 collision = intersect_ray_sphere_simple(r, (Sphere){pos, 0.5f});
        if (collision && global_platform.right_mouse_down || last_entity_pressed == i)
        {
            //sprintf(error_log, "good collision, mouse_dt = %f, %f", global_platform.mouse_dt.x, global_platform.mouse_dt.y);
            last_entity_pressed = i;
            vec3 right = vec3_normalize(vec3_cross(rend->cam.front,rend->cam.up));
            vec3 up = vec3_normalize(rend->cam.up);
            pb->force.x += vec3_mulf(right, 13000 * ((-1.f)*(f32)global_platform.mouse_dt.x / global_platform.window_width)).x;
            pb->force.y += vec3_mulf(up,13000 * ((f32)1.f * global_platform.mouse_dt.y / global_platform.window_height)).y;
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

internal void mouse_pick(ModelManager *model_manager, Renderer *rend)
{

    Ray r =  (Ray){rend->cam.pos, 1, v3(0,0,0)};
    r.d = get_ray_dir(v2(global_platform.mouse_x, global_platform.mouse_y),global_platform.window_width, global_platform.window_height, rend->view, rend->proj);//(Ray){v3(0,0,0), 1, v3(0,0,-1)};
    for (u32 i = 0; i < model_manager->next_index; ++i)
    {
        vec3 pos = v3(model_manager->models[i].model.elements[3][0], model_manager->models[i].model.elements[3][1], model_manager->models[i].model.elements[3][2]);
        i32 collision = intersect_ray_sphere_simple(r, (Sphere){pos, 0.f});
        if (collision && global_platform.right_mouse_down|| last_entity_pressed == i)
        {
            //sprintf(error_log, "good collision, mouse_dt = %f, %f", global_platform.mouse_dt.x, global_platform.mouse_dt.y);
            last_entity_pressed = i;
            vec3 right = vec3_normalize(vec3_cross(rend->cam.front,rend->cam.up));
            vec3 up = vec3_normalize(rend->cam.up);
            model_manager->models[i].model.elements[3][0] += vec3_mulf(right, 100 * ((-1.f)*(f32)global_platform.mouse_dt.x / global_platform.window_width)).x;
            model_manager->models[i].model.elements[3][1] += vec3_mulf(up,100 * ((f32)1.f * global_platform.mouse_dt.y / global_platform.window_height)).y;
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
    mouse_pick(&manager->model_manager, rend);

    for (u32 i = 0; i < manager->model_manager.next_index; ++i)
    {
        SimplePhysicsBody *current = manager->model_manager.models[i].physics_body;
        if (current == NULL)continue;
        //update the _model's_ position
        manager->model_manager.models[i].model.elements[3][0] = current->position.x;
        manager->model_manager.models[i].model.elements[3][1] = current->position.y;
        manager->model_manager.models[i].model.elements[3][2] = current->position.z;

    }
}
internal void 
entity_manager_render(EntityManager *manager, Renderer *rend)
{
    //this is the directional light orientation TODO: stabilize this with real orient
    if (last_entity_pressed >= 0)
    {
        mat4 model = manager->model_manager.models[last_entity_pressed].model;
        vec3 pos = v3(model.elements[3][0], model.elements[3][1], model.elements[3][2]);
        renderer_push_compass(rend, pos);
    }
    for (u32 i = 0; i < manager->model_manager.next_index; ++i)
    {
        renderer_push_model(rend, &manager->model_manager.models[i]);
        SimplePhysicsBody *pb = manager->model_manager.models[i].physics_body;
        if(pb == NULL)continue;
        if (pb->collider.type == BOX)
        {
            renderer_push_cube_wireframe(rend, manager->model_manager.models[i].physics_body->collider.box.min,manager->model_manager.models[i].physics_body->collider.box.max);
            //OBB obb = aabb_to_obb(manager->model_manager.models[i].physics_body->collider.box);
            //renderer_push_obb_wireframe(rend, obb.center, obb.u, obb.e);
        }
        else if (pb->collider.type == ORIENTED_BOUNDED_BOX)
        {
            renderer_push_obb_wireframe(rend, pb->collider.obb.center, pb->collider.obb.u, pb->collider.obb.e);
            //AABB bounded = obb_to_aabb(pb->collider.obb);
            //renderer_push_cube_wireframe(rend, bounded.min, bounded.max);
        }
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
            mat4 model_matrix = mat4_mul(mat4_translate(pos), mat4_mul(mat4_rotate(0.0, v3(1,1,0)), mat4_scale(scale)));
            model_init_cube(m, model_matrix);
           
            m->physics_body = entity_add_rigidbody(&manager->simworld,entity_create(manager));
            *(m->physics_body) = simple_physics_body_default();
            m->physics_body->collider = simple_collider_init(pos,scale);
            m->physics_body->gravity_scale = 1.f;

            m->physics_body->position = pos;
            if (scale.x * scale.y * scale.z > 5.f)
            {
                m->physics_body->mass_data = mass_data_init(0.f);
                m->physics_body->gravity_scale = 0.f;
            }
            else
            {
                m->physics_body->mass_data = mass_data_init(1.f);
                //m->physics_body->mat.restitution = 0.9f;
            }
        }
        else if (strcmp("OBB", str) == 0)
        {
            fscanf(file,"%f", &angle);
            fscanf(file,"%f %f %f", &axis.x, &axis.y, &axis.z);
            m = entity_add_model(&manager->model_manager,entity_create(manager));
            //mat4 model_matrix = mat4_mul(mat4_translate(pos),mat4_mul(mat4_rotate(angle, axis), mat4_scale(scale)));
            mat4 model_matrix = mat4_scale(scale);
            model_init_cube(m, model_matrix);
            m->model = mat4_mul(mat4_translate(pos),mat4_mul(mat4_rotate(angle, axis), mat4_scale(scale)));
           
            m->physics_body = entity_add_rigidbody(&manager->simworld,entity_create(manager));
            *(m->physics_body) = simple_physics_body_default();
            //OBB initialization
            {
                mat4 rotation_matrix = mat4_rotate(angle, vec3_normalize(axis));
                mat3 axes = mat4_extract_rotation(rotation_matrix);

                vec3 center = pos;
                vec3 hw = v3(scale.x,scale.y,scale.z);
                OBB test_obb = obb_init(pos, (f32*)axes.elements, hw);
                m->physics_body->collider.type = ORIENTED_BOUNDED_BOX;
                m->physics_body->collider.obb = test_obb; 
            }
            m->physics_body->gravity_scale = 0.f;

            m->physics_body->position = pos;
            if (scale.x * scale.y * scale.z > 5.f)
            {
                m->physics_body->mass_data = mass_data_init(0.f);
            }
            else
            {
                m->physics_body->mass_data = mass_data_init(1.f);
                //m->physics_body->mat.restitution = 0.9f;
            }
            m->physics_body->collider.type = ORIENTED_BOUNDED_BOX;
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
            m->physics_body->position = pos;
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
