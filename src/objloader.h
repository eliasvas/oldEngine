#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "stdlib.h"
#include "stdio.h"
#include "tools.h"
#include "texture.h"

typedef struct Vertex
{
   vec3 position; 
   vec3 normal;
   vec2 tex_coord;
}Vertex;

typedef struct MeshMaterial
{
  char name[32];
  Texture diff;
  Texture spec;
  f32 shininess;
}MeshMaterial;

typedef struct MeshInfo
{
    Vertex *vertices;
    u32 vertices_count;
    //Material m;
    MeshMaterial material;

}MeshInfo;



static Vertex
vert(vec3 p, vec3 n, vec2 t)
{
    Vertex v;
    v.position = p;
    v.normal = n;
    v.tex_coord = t;
    return v;
}

static u32 count_vertices(char *filename)
{
    u32 count = 0;
    FILE* file = fopen(filename, "r");
	if (file == NULL)
	{
        memcpy(error_log, "Cant Find OBJ!!", 17);
        return 0;
	}
	
    char line[128];
	while(TRUE)
	{
		i32 res = fscanf(file, "%s", line);
		if (res == EOF)break;
	    if (strcmp(line, "f") == 0)count+=3;	
    }
	return count;	
}

static u32 count_different_materials(char *filename)
{
    u32 count = 0;
    FILE* file = fopen(filename, "r");
	if (file == NULL)
	{
        memcpy(error_log, "Cant Find OBJ!!", 17);
        return 0;
	}
	
    char line[128];
	while(TRUE)
	{
		i32 res = fscanf(file, "%s", line);
		if (res == EOF)break;
	    if (strcmp(line, "usemtl") == 0)count++;	
    }
	return count;	
}


static MeshInfo 
obj_load(char *filename){
    MeshInfo res;
    res.vertices_count = count_vertices(filename); 
    Vertex *vertices = (Vertex*)arena_alloc(&global_platform.permanent_storage, sizeof(Vertex) * res.vertices_count);
    u32 vertex_index = 0;

    //at most they will have vertices_count vec3's 
    vec3 *positions = (vec3*)arena_alloc(&global_platform.frame_storage, sizeof(vec3) * res.vertices_count);
    u32 positions_count = 0;
    vec3 *normals = (vec3*)arena_alloc(&global_platform.frame_storage, sizeof(vec3) * res.vertices_count);
    u32 normals_count = 0;
    vec2 *tex_coords= (vec2*)arena_alloc(&global_platform.frame_storage, sizeof(vec2) * res.vertices_count);
    u32  tex_coords_count = 0;

	FILE *file = fopen(filename, "r");
	if (file == NULL)
	{
        memcpy(error_log, "Cant Find OBJ!!!", 17);
        return res;
	}
	
	while(TRUE)
	{
		char line[128];
		i32 res = fscanf(file, "%s", line);
		if (res == EOF)break;
		
		if (strcmp(line, "v") == 0)
		{
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            positions[positions_count++] = vertex;
		}else if (strcmp(line, "vt") == 0)
		{
			vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
            tex_coords[tex_coords_count++] = uv;
		}else if (strcmp(line, "vn") == 0)
		{
			vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            normals[normals_count++] = normal;
		}else if (strcmp(line, "f") == 0) //NOTE(ilias): maybe ditch NEWLINE?? <<
		{
            i32 positions_index[3],normals_index[3], uvs_index[3];
			fscanf(file, "%i/%i/%i %i/%i/%i %i/%i/%i\n", &positions_index[0],&uvs_index[0], &normals_index[0], 
                                                         &positions_index[1],&uvs_index[1], &normals_index[1], 
                                                         &positions_index[2],&uvs_index[2], &normals_index[2]);	

            //NOTE(ilias): we subtract 1 from each index because OBJ indexes start at 1 not 0
             
            Vertex to_add = vert(positions[positions_index[0]-1], normals[normals_index[0]-1], tex_coords[uvs_index[0]-1]);
            vertices[vertex_index++] = to_add;
            to_add = vert(positions[positions_index[1]-1], normals[normals_index[1]-1], tex_coords[uvs_index[1]-1]);
            vertices[vertex_index++] = to_add;
            to_add = vert(positions[positions_index[2]-1], normals[normals_index[2]], tex_coords[uvs_index[2]-1]);
            vertices[vertex_index++] = to_add;
        }
    }
    res.vertices = vertices;

    return res;
}



//We fill the material * and return how many materials we read 
internal u32 mtl_count(char *mtl_filepath)
{
  u32 material_count = 0;
  FILE* file = fopen(mtl_filepath, "r");
	if (file == NULL)
	{
        memcpy(error_log, "Cant Find MTL!!", 17);
        return 0;
	}
	
  char line[128];
  //first we read how many UNIQUE materials there are in the .mtl file
	while(TRUE)
	{
		i32 res = fscanf(file, "%s", line);
		if (res == EOF)break;
	    if (strcmp(line, "newmtl") == 0)++material_count;	
  }
  return material_count;
}

//We fill the material * and return how many materials we read 
internal void mtl_read(char *mtl_filepath, MeshMaterial *materials)
{
  u32 material_count = 0;
  FILE* file = fopen(mtl_filepath, "r");
	if (file == NULL)
	{
        memcpy(error_log, "Cant Find MTL!!", 17);
        return 0;
	}
	
  char line[128];
  //we populate our array with the material info!
  u32 material_offset = 0;
	while(TRUE)
	{
		i32 res = fscanf(file, "%s", line);
		if (res == EOF)break;
	    if (strcmp(line, "newmtl") == 0)
      {
        fscanf(file, "%s", line);
        materials[material_offset].shininess = 256.f;
        memcpy(&materials[material_offset].name, line, str_size(line));
        material_offset++;
        while (TRUE)
        {
          fscanf(file, "%s", line);
            if (strcmp(line, "map_Kd") == 0)
            {
              fscanf(file, "%s", line);
              char diff[32];
              memcpy(diff, line, str_size(line));
              break;
            }
        }
      }  
  }


  return material_count;
}



internal u32 obj_count_meshes(char *objpath)
{
  FILE* file = fopen(objpath, "r");
  u32 meshes_count = 0;
	if (file == NULL)
	{
        memcpy(error_log, "Cant Find MTL!!", 17);
        return 0;
	}
  char line[128];
	while(TRUE)
	{
		i32 res = fscanf(file, "%s", line);
		if (res == EOF)break;
	    if (strcmp(line, "usemtl") == 0)++meshes_count;
  }
}


internal MeshInfo ** obj_read(char *objpath, MeshMaterial *materials)
{

  MeshInfo **meshes;
  u32 vertices_count = count_vertices(objpath);
  Vertex *vertices = (Vertex*)arena_alloc(&global_platform.frame_storage, sizeof(Vertex) * vertices_count);
  u32 vertex_index = 0;

  //at most they will have vertices_count vec3's 
  vec3 *positions = (vec3*)arena_alloc(&global_platform.frame_storage, sizeof(vec3) * vertices_count);
  u32 positions_count = 0;
  vec3 *normals = (vec3*)arena_alloc(&global_platform.frame_storage, sizeof(vec3) * vertices_count);
  u32 normals_count = 0;
  vec2 *tex_coords= (vec2*)arena_alloc(&global_platform.frame_storage, sizeof(vec2) * vertices_count);
  u32  tex_coords_count = 0;

	FILE *file = fopen(objpath, "r");
	if (file == NULL)
	{
        memcpy(error_log, "Cant Find OBJ!!!", 17);
        return NULL;
	}
	
  char line[128];
	while(TRUE)
	{
		i32 res = fscanf(file, "%s", line);
		if (res == EOF)break;
		
		if (strcmp(line, "v") == 0)
		{
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            positions[positions_count++] = vertex;
		}else if (strcmp(line, "vt") == 0)
		{
			vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
            tex_coords[tex_coords_count++] = uv;
		}else if (strcmp(line, "vn") == 0)
		{
			vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            normals[normals_count++] = normal;
		}
    if (strcmp(line, "f") == 0) //NOTE(ilias): maybe ditch NEWLINE?? <<
    {
            i32 positions_index[3],normals_index[3], uvs_index[3];
      fscanf(file, "%i/%i/%i %i/%i/%i %i/%i/%i\n", &positions_index[0],&uvs_index[0], &normals_index[0], 
                                                         &positions_index[1],&uvs_index[1], &normals_index[1], 
                                                         &positions_index[2],&uvs_index[2], &normals_index[2]);	

            //NOTE(ilias): we subtract 1 from each index because OBJ indexes start at 1 not 0
             
            Vertex to_add = vert(positions[positions_index[0]-1], normals[normals_index[0]-1], tex_coords[uvs_index[0]-1]);
            vertices[vertex_index++] = to_add;
            to_add = vert(positions[positions_index[1]-1], normals[normals_index[1]-1], tex_coords[uvs_index[1]-1]);
            vertices[vertex_index++] = to_add;
            to_add = vert(positions[positions_index[2]-1], normals[normals_index[2]], tex_coords[uvs_index[2]-1]);
            vertices[vertex_index++] = to_add;
    }
    else if (strcmp(line, "f") == 0)break;
  }


  return meshes;
}



#endif
