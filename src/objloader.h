#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

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


typedef struct MeshInfo
{
    Vertex *vertices;
    u32 vertices_count;
    //Material m;
}MeshInfo;


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







#endif
