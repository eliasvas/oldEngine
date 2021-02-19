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
    b32 indexed;
    //Material m;
    MeshMaterial material;
    GLuint vao;
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

//We fill the material * and return how many materials we read 
internal u32 mtl_count(char *mtl_filepath)
{
  u32 material_count = 0;
  FILE* file = fopen(mtl_filepath, "r");
	if (file == NULL)
	{
        sprintf(error_log, "cant find mtl: %s", mtl_filepath);
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
        texture_load(&(materials[material_offset].spec),"../assets/white.tga");
        //texture_load(&(materials[material_offset].diff),"../assets/arena/main3.tga");
 
        memcpy(&materials[material_offset].name, line, str_size(line)+1);
        while (TRUE)
        {
          fscanf(file, "%s", line);
            if (strcmp(line, "map_Kd") == 0)
            {
              fscanf(file, "%s", line);
              char diff[64];
              u32 file_index = 0;
              for (u32 i = str_size(mtl_filepath); i>=0;--i)
              {
                if (mtl_filepath[i] == '/')
                {
                  file_index = i; 
                  break;
                }
              }
              memcpy(diff, mtl_filepath,file_index+1);
              memcpy(diff + file_index+1, line, str_size(line)+1);
              texture_load(&(materials[material_offset].diff),diff);
              //sprintf(error_log, "%s", diff);
              break;
            }
        }
        material_offset++;
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
  return meshes_count;
}
internal GLuint 
mesh_gen_vao(u32 start,u32 end, Vertex *vertices)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao); 
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * ((end - start)*3), &vertices[start*3], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *) (sizeof(float) * 3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void *) (sizeof(float) * 6));
    glBindVertexArray(0);

      
    return vao;
}



internal MeshInfo *obj_read(char *objpath, MeshMaterial *materials)
{

  MeshInfo *meshes;

  u32 different_meshes = count_different_materials(objpath);
  meshes = ALLOC(sizeof(MeshInfo) * different_meshes);
  u32 vertices_count = count_vertices(objpath);
  //change this to PERMANENT STORAGE in case we need the vertices for more than one frame
  Vertex *vertices = (Vertex*)arena_alloc(&global_platform.frame_storage, sizeof(Vertex) * vertices_count);
  u32 vertex_index = 0;

  //at most they will have vertices_count vec3's 
  vec3 *positions = (vec3*)arena_alloc(&global_platform.frame_storage, sizeof(vec3) * vertices_count);
  u32 positions_count = 0;
  vec3 *normals = (vec3*)arena_alloc(&global_platform.frame_storage, sizeof(vec3) * vertices_count);
  u32 normals_count = 0;
  vec2 *tex_coords= (vec2*)arena_alloc(&global_platform.frame_storage, sizeof(vec2) * vertices_count);
  u32  tex_coords_count = 0;

  u32 faces_count = 0;
  u32 faces_start = 0;
  i32 current_mesh = -1;

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
    //which means there is a mesh that has just finished giving its faces, we construct it
		if (res == EOF && faces_count != faces_start)
    {
        meshes[current_mesh].vertices_count = (faces_count- faces_start) *3;
        meshes[current_mesh].vertices = &vertices[faces_start];
        meshes[current_mesh].vao = mesh_gen_vao(faces_start,faces_count, vertices);
        faces_start = faces_count*3+1;
        break;

    }
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
    else if (strcmp(line, "f") == 0) //NOTE(ilias): maybe ditch NEWLINE?? <<
    {
            ++faces_count;
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
    }else if (strcmp(line, "usemtl") == 0)
    {
      if (faces_count != 0)
      {
        meshes[current_mesh].vertices_count = (faces_count- faces_start) *3;
        meshes[current_mesh].vertices = &vertices[faces_start*3];
        meshes[current_mesh].vao = mesh_gen_vao(faces_start,faces_count, vertices);
        faces_start = faces_count;
      }
      current_mesh++;
      char mtl_name[32];
      fscanf(file, "%s",mtl_name);
      MeshMaterial found = {0};
      for (u32 i = 0; i < 32;++i) //TODO this is BAAAD why 32
       if (strcmp(materials[i].name, mtl_name) == 0)
       {
           found = materials[i]; 
           break;
       }
      meshes[current_mesh].material = found;
    }
  }


  return meshes;
}


internal MeshInfo *gen_uv_sphere(void);

#endif
