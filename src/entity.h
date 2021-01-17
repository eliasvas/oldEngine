#ifndef ENTITY_H
#define ENTITY_H
#include "tools.h"

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

typedef struct EntityManager
{
    Entity next_entity;
    PositionManager position_manager;
}EntityManager;

internal void 
entity_manager_init(EntityManager *manager)
{
    manager->next_entity = 0;
    position_manager_init(&manager->position_manager);
}
internal Entity 
entity_create(EntityManager *manager)
{
   return ++manager->next_entity; 
}

#endif
