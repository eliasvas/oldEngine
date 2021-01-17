#ifndef COLLADA_PARSER_H
#define COLLADA_PARSER_H
#include "tools.h"
#include "stdio.h"


typedef Vertex vertex ;

static b32 
vert_equals(vertex l, vertex r)
{
    i32 res = ( equalf(l.position.x, r.position.x, 0.001f) && equalf(l.position.y, r.position.y, 0.001f) && equalf(l.position.z, r.position.z, 0.001f)  && equalf(l.normal.x, r.normal.x, 0.001f) && equalf(l.normal.y, r.normal.y, 0.001f) && equalf(l.normal.z, r.normal.z, 0.001f) && equalf(l.tex_coord.x, r.tex_coord.x, 0.001f) && equalf(l.tex_coord.y, r.tex_coord.y, 0.001f));
    return res;
}

#include "animation.h"

static MeshData 
read_collada_maya(String filepath)
{
   MeshData data = {0}; 
   //read the goddamn data
   FILE* file = fopen(filepath.data, "r");
   if (file == NULL)
   {
       sprintf(error_log, "Error Opening .dae file!!\n");
       return (MeshData){0};
   }
   char line[256];
   char garbage[256];
   char count[256];

   while (TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data; //we reached the end of the file
        //we find the <library_geometries> subsection of the collada file
        if (strcmp(line, "<library_geometries>") == 0)
        {
            break;
        }
   }

   //NOTE(ilias): first we read the mesh positions array
   while (TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data; //we reached the end of the file
            //first we read the mesh positions array
        if (strcmp(line, "<float_array") == 0)
        {
            fscanf(file, "%s  %s", garbage, count);
            //count now has the count of the positions to come in the form 'count="2424"'
            i32 position_count = get_num_from_string(count);
            //make the position array of size position_count
            data.positions = (vec3*)arena_alloc(&global_platform.permanent_storage, position_count*sizeof(f32)); //position count is number of floats?? (yes)
            //now read the position data and put them on the array
            vec3 vec;
            for (i32 i = 0; i < position_count/3; ++i)
            {
                fscanf(file, "%f %f %f", &vec.x, &vec.y, &vec.z);
                data.positions[i] = vec;
            }
            break;
        }
   }
   while(TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data; //we reached the end of the file

        //then the normals
        if (strcmp(line, "<float_array") == 0)
        {
            fscanf(file, "%s %s", garbage, count);
            //count has the number of normals     
            i32 normal_count = get_num_from_string(count);
            //make the normal array of size normal_count
            data.normals = (vec3*)arena_alloc(&global_platform.permanent_storage, normal_count*sizeof(f32));
            //read the count data and put the on the array
            vec3 vec;
            for (i32 i = 0; i < normal_count/3; ++i)
            {
                fscanf(file, "%f %f %f", &vec.x, &vec.y, &vec.z);
                data.normals[i] = vec;
            }
            break;
        }
   }
   while(TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data; //we reached the end of the file

        //then the uv coordinates
        if (strcmp(line, "<float_array") == 0)
        {
            fscanf(file, "%s %s", garbage, count);
            //count the number of tex_coords
            i32 tex_count = get_num_from_string(count);
            //make the tex_coords array of size normal count
            data.tex_coords = (vec2*)arena_alloc(&global_platform.permanent_storage, tex_count * sizeof(f32)); 
            //read the tex_coords and put them in the array
            vec2 vec;
            for (i32 i = 0; i < tex_count/2; ++i)
            {
                fscanf(file, "%f %f", &vec.x, &vec.y);
                data.tex_coords[i] = vec;
            }
            break;
        }
   }


   //now read the vertex indices
   vertex* verts;
    while(TRUE)
    {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data; //we reached the end of the file

        if ((strcmp(line, "<triangles") == 0) || strcmp(line, "<polygons") == 0)
        {
            fscanf(file, "%s %s",count, garbage);
            //count the triangles
            data.vertex_count = get_num_from_string(count) * 3;
            //make the vertex array triangle_count triangles big
            verts = (vertex*)arena_alloc(&global_platform.permanent_storage, sizeof(vertex) * data.vertex_count);
            data.verts = (vertex*)arena_alloc(&global_platform.permanent_storage, sizeof(vertex) * data.vertex_count);
            break;
        }
    }

    ivec3 * triangles = (ivec3*)arena_alloc(&global_platform.permanent_storage, sizeof(i32)*3 * data.vertex_count);
    i32 * skinning_index = (i32*)arena_alloc(&global_platform.permanent_storage, sizeof(i32) *data.vertex_count);

    //now read the vertex data
   while(TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)
            return data; //we reached the end of the file

        if (strcmp(line, "<p>") == 0)
        {
            i32 vec[3];
            for (i32 i = 0; i < data.vertex_count; ++i)
            {
                fscanf(file, "%i %i", &vec[0], &vec[1]);
                triangles[i] = (ivec3){vec[0], 0,vec[1]}; 

                //fscanf(file, "%i %i %i", &vec[0], &vec[1], &vec[2]);
                //triangles[i] = (ivec3){vec[0], vec[1], vec[2]}; 
            }
            break;
        }
   }

    for (u32 i = 0; i < data.vertex_count; ++i)
    {
        data.verts[i] = vert(data.positions[triangles[i].x], vec3_normalize(data.normals[triangles[i].x]), data.tex_coords[triangles[i].z]);
        skinning_index[i] = triangles[i].x;
    }
    rewind(file);

    //search for <source = "_Joints_"
   while (TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data; //we reached the end of the file
        //we find the <library_geometries> subsection of the collada file
        if (strstr(line, "Joints") || strstr(line, "joints"))
        {
            break;
        }
   }

   //read the joint names array
   i32 joints_count;
   String* joint_names;
   while (TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data;// we reached the end of the file

        if (strcmp(line, "<Name_array") == 0)
        {
            fscanf(file, "%s", garbage);
            fscanf(file, "%s", count);
            joints_count = get_num_from_string(count);
            joint_names = (String*)arena_alloc(&global_platform.frame_storage, sizeof(String) * joints_count);
            for (i32 i = 0; i< joints_count; ++i)
            {
                fscanf(file, "%s", line);
                joint_names[i] = str(&global_platform.frame_storage, line);
            }
            break;
        }
   }
   data.joint_count = joints_count;
   data.joints = (Joint *)malloc(sizeof(Joint) * data.joint_count*2);
    rewind(file);

   //now lets read the inv bind transforms
   //TRANSFORMS SHOULD BE CONCATINATED (i think)
   mat4* transforms;
   while(TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data; //we reached the end of the file

        if (strstr(line, "-Matrices") || strstr(line, "ProxyController-Matrices") || strstr(line, "boyController-Matrices") || strstr(line, "bind_poses"))
        {
            fscanf(file, "%s %s %s", garbage,garbage, count);
            //count the number of tex_coords
            i32 floats_count = get_num_from_string(count);
            data.transforms_count = floats_count;
            //make the tex_coords array of size normal count
            transforms = (mat4*)arena_alloc(&global_platform.permanent_storage, floats_count * sizeof(f32)); 
            //read the tex_coords and put them in the array
            mat4 mat;
            //for (i32 i = 0; i < floats_count/16; ++i)
            for (i32 i = 0; i < floats_count/16; ++i)
            {
                fscanf(file, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", &mat.raw[0],&mat.raw[1],&mat.raw[2],&mat.raw[3],&mat.raw[4],&mat.raw[5],&mat.raw[6],&mat.raw[7],&mat.raw[8],&mat.raw[9],&mat.raw[10],&mat.raw[11],&mat.raw[12],&mat.raw[13],&mat.raw[14],&mat.raw[15]);

                //NOTE: we transpose because the matrices are given in row major order!!
                mat = mat4_transpose(mat);
                //mat = maya_to_opengl(mat);
                transforms[i] = mat;
            }
            break;
        }
   }
   data.transforms = transforms;

   rewind(file);
   //NOTE(ilias): to find weight info first we find <source id = "_Weights_"
   while (TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data;
        if (strstr(line, "Weights") || strstr(line, "skin-weights"))
        {
            break;
        }
   }


   //now lets read the weights
   i32 weights_count;
   while (TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data;// we reached the end of the file

        if (strcmp(line, "<float_array") == 0)
        {
            fscanf(file, "%s", line);
            //if we are on the weights array
            if (strstr(line, "Weights") || strstr(line, "skin-weights"))
            {
                //we read the count of weights
                fscanf(file, "%s ", count);
                weights_count = get_num_from_string(count);
                //weights = (f32*)arena_alloc(&global_platform.frame_storage, weights_count * sizeof(f32));
                data.weights = (f32*)arena_alloc(&global_platform.permanent_storage, weights_count * sizeof(f32));
                //there is a fake 1.0 at the start of weights array which we eat
                //fscanf(file, "%f", &data.weights[0]); 
                for (i32 i = 0; i < weights_count; ++i)
                {
                   fscanf(file, "%f", &data.weights[i]); 
                }
                break;
            }
        }
   }

   //now lets read the vcount table
   i32* vcount = NULL;
   b32 fin = 0;
   //the total number of weights for all vertices(?)
   i32 vertex_weights_count;
   u32 vcount_sum = 0;
   while(!fin)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data;

        //we are inside the vertex weights source
        if (strcmp(line, "<vertex_weights") == 0)
        {
           fscanf(file, "%s", count); 
           vertex_weights_count = get_num_from_string(count);
           //vcount = (i32*)arena_alloc(&global_platform.frame_storage, sizeof(i32) * vertex_count);
           vcount = (i32*)arena_alloc(&global_platform.permanent_storage, sizeof(i32) * vertex_weights_count);
           while(!fin)
           {
               fscanf(file, "%s", line);
               //now we start reading the vertex number of weights
               if (strcmp(line,"<vcount>") == 0)
               {
                    for(i32 i = 0; i < vertex_weights_count; ++i)
                    {
                        fscanf(file, "%i", &vcount[i]);
                        vcount_sum += vcount[i];
                    }
                    fin = 1;
               }
           }
        }
   }
    

   ivec3* vertex_joint_ids = (ivec3*)arena_alloc(&global_platform.frame_storage, sizeof(ivec3) * data.vertex_count);
   vec3* vertex_weights = (vec3*)arena_alloc(&global_platform.frame_storage, sizeof(vec3) * data.vertex_count);
   while (TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data;// we reached the end of the file
        
        //we found the array of Joint/Weights
        if (strcmp(line, "<v>") == 0)
        {
            i32 JW[2];
            for (i32 i = 0; i < weights_count; ++i)
            {
                vertex_weights[i] =v3(0.f,0.f,0.f);
                for(i32 j = 0; j < vcount[i]; ++j)
                {
                    fscanf(file, "%i %i", &JW[0], &JW[1]);
                    f32 weight = data.weights[JW[1]];
                    //we fill the vertex joint/weight tables with the max weights
                    if (j == 0)
                    {
                       vertex_weights[i].x = weight;
                       vertex_joint_ids[i].x = JW[0];
                    }else if (j == 1)
                    {
                        vertex_weights[i].y = weight;
                        vertex_joint_ids[i].y = JW[0];
                    }else if (j == 2)
                    {
                        vertex_weights[i].z = weight;
                        vertex_joint_ids[i].z = JW[0];
                    }
                    //vertex_weights[i].x += 1.f - vertex_weights[i].x - vertex_weights[i].y - vertex_weights[i].z;
                }
            }
            break;
        }
   }
   //now lets make the final AnimatedVertex array
   data.vertices = (AnimatedVertex*)arena_alloc(&global_platform.permanent_storage, sizeof(AnimatedVertex) * 3 * data.vertex_count);
   for (u32 i = 0; i < data.vertex_count; ++i)
   {
       //the skinning_index table is used as the 'index' to the vertex joint and vertex_weight arrays which are supposed to be
       //indexed by the index of the position which can be found on the <vertices> group 
       data.vertices[i] = (AnimatedVertex){data.verts[i].position, data.verts[i].normal, data.verts[i].tex_coord, vertex_joint_ids[skinning_index[i]], vertex_weights[skinning_index[i]]};
   }

   //we find the root joint(s) (multiple root joints are not currently supported)
   //by reading the first <skeleton> type string
   String root_name;
   while (TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)
        {
            //root_name = str(&global_platform.permanent_storage, "root");
            root_name = str(&global_platform.permanent_storage, joint_names[0].data);
            break;
        }
        
        if (strcmp(line, "<skeleton>") == 0)
        {
            fscanf(file, "%s", line); //name of the root joint

            root_name = substr(&global_platform.frame_storage, line, 1, str_size(line)); 
            break;
        }
   }
   rewind(file);

   //now we know the root we should read the original transforms of every joint
   //we do this by reading the <library_visual_scenes> <node>s
   //if the node is of type "JOINT" it means its a child node..
   //if its of type "NODE" its a new root (<skeleton> thing)

   //as for the hierarchy of joints.. each <node we read is a child of the previous node that has
   //not been closed (by </node>)
  //Joint *root = arena_alloc(&global_platform.permanent_storage, sizeof(Joint));
  
   u32 nodes_open = 0;
   u32 fake_nodes_open = 0;
   mat4 mat;
   //we read until <library_visual_scenes>
   while (TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data;
        
        if (strcmp(line, "<library_visual_scenes>") == 0)
        {
            fscanf(file, "%s %s %s %s",garbage, garbage, garbage, garbage);
            break;
        }
   }

   //we see whether our <node is the root (by strcmp!)
   i32 root_index;
   while (TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return data;
        
        if (strcmp(line, "<node") == 0)
        {
            fscanf(file, "%s", line);
            String name = substr(&global_platform.frame_storage, line, 6,str_size(line) -1); 
            root_name = name;
            //read root data
            if (strcmp(name.data, root_name.data) == 0)
            {
                fscanf(file, "%s", line);
                String id = substr(&global_platform.permanent_storage, line, 4,str_size(line) -1); 
                fscanf(file, "%s", line);
                String sid = substr(&global_platform.permanent_storage, line, 5,str_size(line) -1);  
                u32 index;
                for(index = 0; index < joints_count; ++index)
                    if (strcmp(joint_names[index].data, sid.data) ==0)
                        break;
                //eat type="__", <matrix, sid ="matrix"
                fscanf(file, "%s %s %s", garbage, garbage, garbage);
                //read the local bind matrix
                fscanf(file, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", &mat.raw[0],&mat.raw[1],&mat.raw[2],&mat.raw[3],&mat.raw[4],&mat.raw[5],&mat.raw[6],&mat.raw[7],&mat.raw[8],&mat.raw[9],&mat.raw[10],&mat.raw[11],&mat.raw[12],&mat.raw[13],&mat.raw[14],&mat.raw[15]);
                mat = mat4_transpose(mat);
                //mat = maya_to_opengl(mat);
               // mat4 local_bind_transform = mat;
                mat4 local_bind_transform = m4d(1.f);
                local_bind_transform = mat;

                //local_bind_transform = transforms[index];
                data.joints[index] = joint_sid(index, id, sid, local_bind_transform);
                data.joints[index].inv_bind_transform = transforms[index];
                data.joints[index].animated_transform = m4d(1.f);
                data.joints[index].parent_id = index;
                root_index = index;
                nodes_open++;
                break;
            }
        }
   }
   u32 current_index = data.joints[root_index].index;
   while (nodes_open > 0)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)
            return data;
        
        if (strcmp(line, "<node") == 0)
        {
            fscanf(file, "%s", line);
            if (strstr(line, "Orient") || strstr(line, "End")){
                fake_nodes_open++;
                continue;
            }
            String name = substr(&global_platform.frame_storage, line, 6,str_size(line) -1); 
            fscanf(file, "%s", line);
            String id = substr(&global_platform.permanent_storage, line, 4,str_size(line) -1);  
            fscanf(file, "%s", line);
            String sid = substr(&global_platform.permanent_storage, line, 5,str_size(line) -1);  
            u32 index;
            for(index = 0; index < joints_count; ++index)
                if (strcmp(joint_names[index].data, sid.data) ==0)
                    break;
            //eat type="__", <matrix, sid ="matrix"
            fscanf(file, "%s",garbage);
            if (!strstr(garbage, "type=\"JOINT\"") || index >= joints_count){
                fake_nodes_open++;
                continue;
            }

            fscanf(file, "%s %s", garbage, garbage);
            //read the local bind matrix
            fscanf(file, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", &mat.raw[0],&mat.raw[1],&mat.raw[2],&mat.raw[3],&mat.raw[4],&mat.raw[5],&mat.raw[6],&mat.raw[7],&mat.raw[8],&mat.raw[9],&mat.raw[10],&mat.raw[11],&mat.raw[12],&mat.raw[13],&mat.raw[14],&mat.raw[15]);
            mat = mat4_transpose(mat);
            //mat = maya_to_opengl(mat);
            mat4 local_bind_transform = m4d(1.f);
            local_bind_transform = mat;
            Joint to_add = joint_sid(index, id, sid, local_bind_transform);
            to_add.inv_bind_transform = transforms[index];
            to_add.animated_transform = m4d(1.f); 
            //to_add.parent = current;
            to_add.parent_id = current_index;
            nodes_open++;
            //current->num_of_children++;
            data.joints[current_index].num_of_children++;
            data.joints[index] = to_add;
            //current->children[current->num_of_children - 1] = to_add;
            //current = &current->children[buf_len(current->children) - 1];
            //current = &current->children[current->num_of_children - 1];
            current_index = index;
        }

        if (strcmp(line, "</node>") == 0)
        {
            if (fake_nodes_open > 0)
            {
                fake_nodes_open--;
                continue;
            }

            nodes_open--;
            //current = current->parent;
            current_index = data.joints[current_index].parent_id;
        }
   }
    //now get the bind shape matrix
    rewind(file);
    while (TRUE)
    {
         i32 res = fscanf(file, "%s", line);
         if (res == EOF)return data;
         
         mat4 mat;
         if (strcmp(line, "<bind_shape_matrix>") == 0)
         {
               fscanf(file, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", &mat.raw[0],&mat.raw[1],&mat.raw[2],&mat.raw[3],&mat.raw[4],&mat.raw[5],&mat.raw[6],&mat.raw[7],&mat.raw[8],&mat.raw[9],&mat.raw[10],&mat.raw[11],&mat.raw[12],&mat.raw[13],&mat.raw[14],&mat.raw[15]);
               mat = mat4_transpose(mat);
               //mat = maya_to_opengl(mat);
               data.bind_shape_matrix = mat;
               break;
         }
    }

   fclose(file);
   return data;
}




static Animation 
read_collada_animation(String filepath) {
   Animation anim;    
   //by default all animations play at normal speed!
   anim.playback_rate = 1.f;
   FILE* file = fopen(filepath.data, "r");
   if (file == NULL)
   {
       sprintf(error_log, "Error opening Collada file: %s", filepath.data);
       return (Animation){0};
   }
   char line[256];
   char garbage[256];
   char count[256];
   u32 number_of_joint_animations = 0;

   //first go FP UNTIL <library_controllers> 
   while (TRUE)
   {
       i32 res = fscanf(file, "%s", line);
       if (res == EOF)return anim;// we reached the end of the file
       if (strcmp(line, "<library_controllers>") == 0)
       {
          break;
       }
   }

   //read the joint names array
   i32 joints_count;
   String* joint_names;
   while (TRUE)
   {
        i32 res = fscanf(file, "%s", line);
        if (res == EOF)return anim;// we reached the end of the file

        if (strcmp(line, "<Name_array") == 0)
        {
            fscanf(file, "%s", garbage);
            fscanf(file, "%s", count);
            joints_count = get_num_from_string(count);
            joint_names = (String*)arena_alloc(&global_platform.frame_storage, sizeof(String) * joints_count);
            for (i32 i = 0; i< joints_count; ++i)
            {
                fscanf(file, "%s", line);
                joint_names[i] = str(&global_platform.frame_storage, line);
            }
            break;
        }
   }



   //go until the start of the first animation
   while (TRUE)
   {
       i32 res = fscanf(file, "%s", line);
       if (res == EOF)return anim;// we reached the end of the file
 
       //count number of joint animations
       if (strcmp(line, "<library_animations>") == 0)
       {
           //<animation id="ABC" name="ABC" <--skeleton data not needed?
           //fscanf(file, "%s %s %s", garbage, garbage, garbage);
           while (res != EOF){
               res = fscanf(file, "%s", line);
               if (strcmp(line, "<animation") == 0)
               {
                   number_of_joint_animations++;
                   fscanf(file, "%s %s %s %s", garbage,garbage, garbage, garbage);
               }
           }
           break;
      }
   }
   anim.joint_animations = (JointAnimation*)arena_alloc(&global_platform.permanent_storage, sizeof(JointAnimation) * number_of_joint_animations);
   anim.joint_anims_count = number_of_joint_animations;
   rewind(file);




   i32 current_joint_animation = 0;
   

   //now lets starting reading the actual animation
   while (TRUE)
   {
       i32 res = fscanf(file, "%s", line);
       if (res == EOF)return anim;// we reached the end of the file
       if (strcmp(line, "<library_animations>") == 0)
       {
          //<animation id="ABC" name="ABC" <--skeleton data not needed?
          break;
       }
   }

   i32 joint_index = 0;
   i32 keyframe_count;
   while (current_joint_animation < number_of_joint_animations)
   {
       i32 res = fscanf(file, "%s", line);
       if (res == EOF)
       {
           fclose(file);
           return anim;// we reached the end of the file
       }
       if (strcmp(line, "<animation") == 0)
       {
           fscanf(file, "%s %s %s %s %s %s %s",garbage, garbage,garbage, garbage, garbage, garbage, garbage);
           char *str = garbage + 4;
           char *end_of_name = strchr(str,'-');
           i32 len = (int)end_of_name - (int)str;
           for (i32 i =0; i <=joints_count; ++i) //bigbrain Bone Bone_01 .... Bone gets popped first..
           {
               if (strncmp(str, joint_names[i].data, max(joint_names[i].len, len)) == 0)
               {
                   joint_index = i;
                   break;
               }
                   
           }
           fscanf(file, "%s", count);
           anim.joint_animations[current_joint_animation].keyframe_count = get_num_from_string(count);
           keyframe_count = anim.joint_animations[current_joint_animation].keyframe_count;
           anim.joint_animations[current_joint_animation].keyframes = (JointKeyFrame*)arena_alloc(&global_platform.permanent_storage,sizeof(JointKeyFrame) * keyframe_count);
           for (u32 i = 0; i < keyframe_count; ++i)
           {
                fscanf(file, "%f", &anim.joint_animations[current_joint_animation].keyframes[i].timestamp);
           }
           mat4 mat;
           //now lets read the matrices for each timestamp
           while (TRUE)
           {
               res = fscanf(file, "%s", line);
               if (strcmp(line, "<float_array") == 0)
               {
                   fscanf(file,"%s %s", garbage, count);
                   for (u32 i = 0; i < keyframe_count; ++i)
                   {
                       fscanf(file, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", &mat.raw[0],&mat.raw[1],&mat.raw[2],&mat.raw[3],&mat.raw[4],&mat.raw[5],&mat.raw[6],&mat.raw[7],&mat.raw[8],&mat.raw[9],&mat.raw[10],&mat.raw[11],&mat.raw[12],&mat.raw[13],&mat.raw[14],&mat.raw[15]);
                       mat = mat4_transpose(mat);
                       //JointTransform t = {v3(mat.elements[3][0],mat.elements[3][1],mat.elements[3][2]), mat4_to_quat(mat)};
                       //JointTransform t = {v3(mat.elements[3][0],mat.elements[3][1],mat.elements[3][2]), quat_normalize(mat4_to_quat(mat)), mat};
                       JointTransform t = {v3(mat.elements[3][0],mat.elements[3][1],mat.elements[3][2]),mat4_to_quat(mat), mat};
                       anim.joint_animations[current_joint_animation].keyframes[i].transform = t;
                       anim.joint_animations[current_joint_animation].keyframes[i].joint_index = joint_index;
                       anim.length = max(anim.length, anim.joint_animations[current_joint_animation].keyframes[keyframe_count - 1].timestamp);
                       anim.joint_animations[current_joint_animation].length= anim.joint_animations[current_joint_animation].keyframes[keyframe_count - 1].timestamp;
                   }
                   break;
               }
           }
           current_joint_animation++;
       }
   }

    fclose(file);
    return anim;
}





static Animator
animator_init(String diffuse_texture, String collada_model, String collada_animation) 
{
        Animator animator;
        Texture *anim_tex = malloc(sizeof(Texture));
        texture_load(anim_tex,diffuse_texture.data);
        //only initialize if not already initialized!
        if (&anim_shader != NULL)
            shader_load(&anim_shader,"../assets/shaders/animated3d.vert", "../assets/shaders/animated3d.frag");
        MeshData dae_data = read_collada_maya(str(&global_platform.permanent_storage,collada_model.data));
        AnimatedModel animated_model = animated_model_init(anim_tex, dae_data.root,&dae_data);
        Animation animation_to_play = read_collada_animation(str(&global_platform.permanent_storage,collada_animation.data));
        Animation *atp = arena_alloc(&global_platform.permanent_storage, sizeof(Animation));
        *atp = animation_to_play;
        JointKeyFrame *prev_pose = arena_alloc(&global_platform.permanent_storage, sizeof(JointKeyFrame) * animated_model.joint_count);
        animator = (Animator){animated_model, atp, 1.05f, prev_pose, 0.f, 1.f};
        return animator;
}



#endif
