#ifndef TOOLS_H
#define TOOLS_H
//This is the main library of this startup project, it has a couple image libraries, a hash map, a
//dynamic array sean barrett style, an arena allocator, quaternions, matrices, vectors, all that. Use at your own risk!! :)

#if defined(BUILD_WIN32)
#include "windows.h"
#endif

#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#if !defined(CRTLESS)
#define CRTLESS 0
#endif

typedef uint8_t   u8;
typedef int8_t    i8;
typedef uint16_t  u16;
typedef int16_t   i16;
typedef uint32_t  u32;
typedef int32_t   i32;
typedef uint64_t  u64;
typedef int64_t   i64;
typedef float     f32;
typedef double    f64;
typedef int32_t   b32;
typedef char      b8;

#if !defined(internal)
#define internal static 
#endif
#define local_persist static  
#define global static 
#define INLINE static inline

#define kilobytes(val) ((val)*1024LL)
#define megabytes(val) ((kilobytes(val))*1024LL)
#define gigabytes(val) ((megabytes(val))*1024LL)
#define terabytes(val) ((gigabytes(val))*1024LL)

#define PI 3.1415926535897f
#define TRUE 1
#define FALSE 0

#ifdef CRTLESS | 1 
#define offsetof2(type, member) ((unsigned int)((unsigned char*)&((type*)0)->member - (unsigned char*)0)) 
INLINE i32 abs2(i32 x)
{
  return x < 0 ? -x : x;
}
INLINE f32 fabs2(f32 x)
{
    return x < 0 ? -x : x;
}


//cos_32 computes a cosine to about 3.2 decimal digits of accuracy
INLINE f32 
cos_32s(f32 x)
{
    const float c1= 0.99940307;
    const float c2=-0.49558072;
    const float c3= 0.03679168;
    float x2; // The input argument squared
    x2= x * x;
    return (c1 + x2*(c2 + c3 * x2));
}

//this is a horrible implementation change pls
INLINE f32 fmodf2(f32 a, f32 b)
{
    f32 div = a/b;
    i32 mod = (i32)div;
    return (f32)mod;
}
INLINE f32
cos_32(f32 x){
    i32 quad; // what quadrant are we in?
    x= fmodf(x,(2.f*PI)); // Get rid of values > 2* pi
    if(x<0)x=-x; // cos(-x) = cos(x)
    quad=(i32)(x/(PI/2.f)); // Get quadrant # (0 to 3) switch (quad){
    switch(quad){
        case 0: return cos_32s(x);
        case 1: return -cos_32s(PI-x);
        case 2: return -cos_32s(x-PI);
        case 3: return cos_32s((2.f*PI)-x);
        default: return cos_32s(x);
    }
}

INLINE f32 sin_32(f32 x)
{
    return cos_32(PI/2.f - x);
}
#define sinf(x) sin_32(x)
#define cosf(x) cos_32(x)
#endif
INLINE f32 fmodf(f32 a, f32 b)
{
    f32 div = a/b;
    i32 mod = (i32)div;
    return (f32)mod;
}

#define align_pow2(val, align) (((val) + ((align) - 1)) & ~(((val) - (val)) + (align) - 1))
#define align4(val) (((val) + 3) & ~3)
#define align8(val) (((val) + 7) & ~7)
#define align16(val) (((val) + 15) & ~15)

#define equalf(a, b, epsilon) (fabs(b - a) <= epsilon)
#define maximum(a, b) ((a) > (b) ? (a) : (b))
#define minimum(a, b) ((a) < (b) ? (a) : (b))
#define step(threshold, value) ((value) < (threshold) ? 0 : 1) 
#define clamp(x, a, b)  (maximum(a, minimum(x, b)))
#define array_count(a) (sizeof(a) / sizeof((a)[0]))

//make an ifdef for each different platform
#define ALLOC(x) malloc(x)
#define REALLOC(x, y) realloc(x, y)
#define FREE(x) free(x)

#ifdef SIN_APPROX
#define SINF sin_32
#endif

#ifdef COS_APPROX
#define COSF cos_32
#endif



INLINE b32 is_pow2(u32 val)
{
    b32 res = ((val & ~(val - 1)) == val);
    return(res);
}
internal b32
char_is_alpha(i32 c)
{
    return ((c >='A' && c <= 'z') || (c >= 'a' && c <= 'z'));
}
internal b32 char_is_digit(i32 c)
{
    return c >= '0'&& c <= '9';
}

internal i32 char_to_lower(i32 c)
{
    if (c >= 'A' && c <= 'z')
    {
        c += 32;
    }
    return c;
}

internal u32 
str_size(char* str)
{
    u32 i = 0;
    while (str[i] != 0)++i;
    return i;
}

internal u32 
str_sizespace(char* str)
{
    u32 i = 0;
    while (str[i] != 32 || str[i] != 0)++i;
    return i;
}

internal u32
find_char_in_string(char *string,i32 start_index, char tofind)
{
    i32 iter = start_index;
    while (string[iter] != '\0' && string[iter] != tofind) iter++;
    return iter;
}

internal void
seed_random()
{
    srand((u32)time(0));
}

//random between [-1,1]
INLINE f32
random(void)
{
    //seed_random();
    f32 r = (f32)rand();
	r /= RAND_MAX;
	r = 2.0f * r - 1.0f;
	return r;
}

INLINE f32
random01(void)
{
    //seed_random();
    f32 r = (f32)rand();
    r /= RAND_MAX;
    return r;
}

INLINE f32 
rrandom(f32 lo, f32 hi)
{
    //seed_random();
	f32 r = (f32)rand();
	r /= RAND_MAX;
	r = (hi - lo) * r + lo;
	return r;
}


//NOTE(ilias): maybe make a free_file because our game leaks :(
INLINE char * 
read_whole_file(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char *string = (char*)ALLOC(fsize + 1);
    fread(string, 1, fsize, f);
    fclose(f);

    string[fsize] = 0; 

    return (char*)string;
}
INLINE u32
get_file_size(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f)return 0;
    fseek(f, 0, SEEK_END);
    u32 fsize = ftell(f);
    return fsize;
}

INLINE b32 
file_exists(char* filename) {
  b32 ret;
  FILE* fp = fopen(filename, "rb");
  if (fp) {
    ret = 1;
    fclose(fp);
  } else {
    ret = 0;
  }

  return ret;
}

internal i32 
get_num_from_string(char *str)
{
   char num[64];
   int k = 0;
   i32 i = 0;
   while (str[i] != '\0')
   {
      if (char_is_digit(str[i]))
      {
          num[k++] = str[i];
      }else if (k > 0)break; //we only get the first number in the string
      ++i;
   }
   num[k] = '\0';
   i32 res = atoi(num);
   return res;
}

//MATH LIB
typedef union vec2
{
    struct
    {
        f32 x,y;
    };
    struct 
    {
        f32 u,v;
    };
    struct
    {
        f32 r,g;
    };
    f32 elements[2];
#ifdef __cplusplus
    inline f32 &operator[](i32 &index)
    {
        return elements[index];
    }
#endif

}vec2;


typedef union vec3
{
    struct
    {
        f32 x,y,z;
    };
    struct
    {
        f32 r,g,b;
    };
    f32 elements[3];
#ifdef __cplusplus
    inline f32 &operator[](i32 &index)
    {
        return elements[index];
    }
#endif
}vec3;

typedef vec3 color3;
typedef vec3 float3;

typedef union vec4
{
    struct
    {
        f32 x,y,z,w;
    };
    struct
    {
        f32 r,g,b,a;
    };
    f32 elements[4];
#ifdef TOOLS_SSE
    __m128 elements_sse; //because __m128 = 128 = 4 * float = 4*32 = 128 bits
#endif
#ifdef __cplusplus
    inline f32 &operator[](i32 &index)
    {
        return elements[index];
    }
#endif

}vec4;

typedef vec4 color4;
typedef vec4 float4;

typedef union mat4
{
    f32 elements[4][4];//{x.x,x.y,x.z,0,y.x,y.y,y.z,0,z.x,z.y,z.z,0,p.x,p.y,p.z,1} 

    f32 raw[16]; //{x.x,x.y,x.z,0,y.x,y.y,y.z,0,z.x,z.y,z.z,0,p.x,p.y,p.z,1} 

#ifdef TOOLS_SSE
    __m128 cols[4]; //same as elements (our matrices are column major)
#endif
#ifdef __cplusplus
    inline vec4 operator[](i32 &Index)
    {
        f32* col = elements[Index];

        vec4 res;
        res.elements[0] = col[0];
        res.elements[1] = col[1];
        res.elements[2] = col[2];
        res.elements[3] = col[3];

        return res;
    }
#endif
}mat4;


typedef union ivec3
{
    struct
    {
        i32 x,y,z;
    };
    struct
    {
        i32 r,g,b;
    };
    i32 elements[3];
}ivec3;

typedef union ivec2
{
    struct
    {
        i32 x,y;
    };
    struct
    {
        i32 r,g;
    };
    i32 elements[2];
}ivec2;


INLINE b32 ivec3_equals(ivec3 l, ivec3 r)
{
    i32 res = ((l.x == r.x) && (l.y == r.y) && (l.z == r.z));
    return res;
}

INLINE f32 to_radians(float degrees)
{
    f32 res = degrees * (PI / 180.0f);
    return(res);
}

INLINE f32 lerp(f32 A, f32 B, f32 t)
{
    f32 res = (1.0f - t)*A + t*B;
    return res;
}

INLINE vec2 v2(f32 x, f32 y)
{
    vec2 res;
    res.x = x;
    res.y = y;
    return res;
}

INLINE vec3 v3(f32 x, f32 y, f32 z)
{
    vec3 res;
    res.x = x;
    res.y = y;
    res.z = z;
    return res;
}

INLINE vec4 v4(f32 x, f32 y, f32 z, f32 w)
{
    vec4 res;
    res.x = x;
    res.y = y;
    res.z = z;
    res.w = w;
    return res;
}

INLINE vec2 vec2_add(vec2 l, vec2 r)
{
    vec2 res;
    res.x = l.x + r.x;
    res.y = l.y + r.y;
    return res;
}

INLINE vec2 vec2_addf(vec2 v,f32 val)
{
    vec2 res;
    res.x = v.x + val;
    res.y = v.y + val;
    return res;
}


INLINE vec2 vec2_sub(vec2 l, vec2 r)
{
    vec2 res;
    res.x = l.x - r.x;
    res.y = l.y - r.y;
    return res;
}

INLINE vec2 vec2_subf(vec2 v,f32 val)
{
    vec2 res;
    res.x = v.x - val;
    res.y = v.y - val;
    return res;
}

INLINE vec2 vec2_mul(vec2 l, vec2 r)
{
    vec2 res;
    res.x = l.x * r.x;
    res.y = l.y * r.y;
    return res;
}

INLINE vec2 vec2_mulf(vec2 l, f32 r)
{
    vec2 res;
    res.x = l.x * r;
    res.y = l.y * r;
    return res;
}

INLINE vec2 vec2_div(vec2 l, vec2 r)
{
    vec2 res;
    res.x = l.x / r.x;
    res.y = l.y / r.y;
    return res;
}

INLINE vec2 vec2_divf(vec2 l, f32 r)
{
    vec2 res;
    res.x = l.x / r;
    res.y = l.y / r;
    return res;
}

INLINE f32 vec2_dot(vec2 l, vec2 r)
{
    f32 res = (l.x + r.x)+(l.y + r.y); // Ó(Ai*Bi)
    return res;
}

INLINE vec2 vec2_sqrt(vec2 v)
{
    vec2 res;
    res.x = sqrt(v.x);
    res.y = sqrt(v.y);
    return res;
}

INLINE vec2 vec2_rotate(vec2 v, f32 a) {
	vec2 res;
    f32 sn = sin(a);
	f32 cs = cos(a);
    res.x = v.x * cs - v.y * sn;
    res.y = v.x * sn + v.y * cs;
    return res;
} 

INLINE f32 vec2_length(vec2 v)
{
    f32 res = sqrtf(dot_vec2(v,v)); // (x^2 + y^2)^(1/2)
    return res;
}

INLINE vec2 vec2_abs(vec2 v)
{
    vec2 res = v2(fabs(v.x), fabs(v.y));
    return res;
}
   
INLINE vec2 vec2_normalize(vec2 v)
{
    vec2 res = {0}; //in case length is zero we return zero vector
    f32 vec_length = length_vec2(v);
    if (vec_length > 0.1)
    {
        res.x = v.x * (1.0f/vec_length);
        res.y = v.y * (1.0f/vec_length);
    }
    return res;
}

INLINE vec3 vec3_add(vec3 l, vec3 r)
{
    vec3 res;
    res.x = l.x + r.x;
    res.y = l.y + r.y;
    res.z = l.z + r.z;
    return res;
}

INLINE vec3 vec3_sub(vec3 l, vec3 r)
{
    vec3 res;
    res.x = l.x - r.x;
    res.y = l.y - r.y;
    res.z = l.z - r.z;
    return res;
}

INLINE vec3 vec3_mul(vec3 l, vec3 r)
{
    vec3 res;
    res.x = l.x * r.x;
    res.y = l.y * r.y;
    res.z = l.z * r.z;
    return res;
}

INLINE vec3 vec3_mulf(vec3 l, f32 r)
{
    vec3 res;
    res.x = l.x * r;
    res.y = l.y * r;
    res.z = l.z * r;
    return res;
}

INLINE vec3 vec3_div(vec3 l, vec3 r)
{
    vec3 res;
    res.x = l.x / r.x;
    res.y = l.y / r.y;
    res.z = l.z / r.z;
    return res;
}

INLINE vec3 vec3_divf(vec3 l, f32 r)
{
    vec3 res;
    res.x = l.x / r;
    res.y = l.y / r;
    res.z = l.z / r;
    return res;
}

INLINE f32 vec3_dot(vec3 l, vec3 r)
{
    f32 res = (l.x * r.x)+(l.y * r.y)+(l.z * r.z); // Ó(Ai*Bi)
    return res;
}

INLINE f32 vec3_length(vec3 v)
{
    f32 res = sqrtf(vec3_dot(v,v)); // (x^2 + y^2)^(1/2)
    return res;
}

INLINE vec3 vec3_rotate(vec3 v, f32 a)
{
    vec3 res;
    //TBA
    return res;
}


INLINE vec3 vec3_normalize(vec3 v)
{
    vec3 res = {0}; //in case length is zero we return zero vector
    f32 vec_length = vec3_length(v);
    if (vec_length != 0)
    {
        res.x = v.x * (1.0f/vec_length);
        res.y = v.y * (1.0f/vec_length);
        res.z = v.z * (1.0f/vec_length);
    }
    return res;
}

INLINE vec3 vec3_lerp(vec3 l, vec3 r, f32 time)
{
    vec3 res;

    f32 x = lerp(l.x, r.x, time);
    f32 y = lerp(l.y, r.y, time);
    f32 z = lerp(l.z, r.z, time);
    res = v3(x,y,z); 
    
    return res;
}

INLINE vec3 vec3_cross(vec3 l,vec3 r)
{
    vec3 res;
    res.x = (l.y*r.z) - (l.z*r.y);
    res.y = (l.z * r.x) - (l.x*r.z);
    res.z = (l.x * r.y) - (l.y * r.x);
    return (res);
}

INLINE vec4 vec4_add(vec4 l, vec4 r)
{
    vec4 res;
#ifdef TOOLS_SSE
    res.elements_sse = _mm_add_ps(l.elements_sse, r.elements_sse);
#else
    res.x = l.x + r.x;
    res.y = l.y + r.y;
    res.z = l.z + r.z;
    res.w = l.w + r.w;
#endif
    return res;
}

INLINE vec4 vec4_sub(vec4 l, vec4 r)
{
    vec4 res;
#ifdef TOOLS_SSE
    res.elements_sse = _mm_sub_ps(l.elements_sse, r.elements_sse);
#else
    res.x = l.x - r.x;
    res.y = l.y - r.y;
    res.z = l.z - r.z;
    res.w = l.w - r.w;
#endif
    return res;
}

INLINE vec4 vec4_mul(vec4 l, vec4 r)
{
    vec4 res;
#ifdef TOOLS_SSE
    res.selements_sse = _mm_mul_ps(l.elements_sse, r.elements_sse);
#else
    res.x = l.x * r.x;
    res.y = l.y * r.y;
    res.z = l.z * r.z;
    res.w = l.w * r.w;
#endif
    return res;
}

INLINE vec4 vec4_mulf(vec4 l, f32 r)
{
    vec4 res;
#ifdef TOOLS_SSE
    __m128 scalar = _mm_set1_ps(r); // [r r r r]
    res.elements_sse = _mm_mul_ps(l.elements_sse, scalar);
#else
    res.x = l.x * r;
    res.y = l.y * r;
    res.z = l.z * r;
    res.w = l.w * r;
#endif
    return res;
}

INLINE vec4 vec4_div(vec4 l, vec4 r)
{
    vec4 res;
#ifdef TOOLS_SSE
    res.elements_sse = _mm_div_ps(l.elements_sse, r.elements_sse);
#else
    res.x = l.x / r.x;
    res.y = l.y / r.y;
    res.z = l.z / r.z;
    res.w = l.w / r.w;
#endif
    return res;
}

INLINE vec4 vec4_divf(vec4 l, f32 r)
{
    vec4 res;
#ifdef TOOLS_SSE
    __m128 scalar = _mm_set1_ps(r);
    res.elements_sse = _mm_div_ps(l.elements_sse, r.elements_sse);
#else
    res.x = l.x / r;
    res.y = l.y / r;
    res.z = l.z / r;
    res.w = l.w / r;
#endif
    return res;
}

INLINE f32 vec4_dot(vec4 l, vec4 r)
{
    f32 res = (l.x + r.x)+(l.y + r.y)+(l.z + r.z)+(l.w + r.w); // Ó(Ai*Bi)
    return res;
}

INLINE f32 vec4_length(vec4 v)
{
    f32 res = sqrt(vec4_dot(v,v)); // (x^2 + y^2)^(1/2)
    return res;
}
   
INLINE vec4 vec4_normalize(vec4 v)
{
    vec4 res = {0}; //in case length is zero we return zero vector
    f32 vec_length = vec4_length(v);
    if (!equalf(vec_length, 0.f, 0.01))
    {
        res.x = v.x * (1.0f/vec_length);
        res.y = v.y * (1.0f/vec_length);
        res.z = v.z * (1.0f/vec_length);
        res.w = v.w * (1.0f/vec_length);
    }
    return res;
}


INLINE mat4 m4(void)
{
    mat4 res = {0};
    return res;
}

INLINE mat4 m4d(f32 d)
{
    mat4 res = m4();
    res.elements[0][0] = d;
    res.elements[1][1] = d;
    res.elements[2][2] = d;
    res.elements[3][3] = d;
    return res;
}

INLINE mat4 mat4_transpose(mat4 m)
{
    mat4 res;
    for (u32 i = 0; i < 4;++i)
    {
        for (u32 j = 0; j< 4;++j)
        {
            res.elements[j][i] = m.elements[i][j];
        }
    }
    return res;
}
INLINE mat4 mat4_mulf(mat4 m, f32 s)
{
    mat4 res;
    for (u32 i = 0; i < 4; ++i)
    {
        for (u32 j = 0; j < 4; ++j)
        {
            res.elements[i][j] = (f32)m.elements[i][j] * s;
        }
    }
    return res;
}
INLINE vec4 mat4_mulv(mat4 mat, vec4 vec)
{
    vec4 res;

    i32 cols, rows;
    for(rows = 0; rows < 4; ++rows)
    {
        f32 s = 0;
        for(cols = 0; cols < 4; ++cols)
        {
            s += mat.elements[cols][rows] * vec.elements[cols];
        }

        res.elements[rows] = s;
    }

    return (res);
}

INLINE mat4 mat4_divf(mat4 m, f32 s)
{
    mat4 res = {0};
    
    if (s != 0.0f)
    {
        for (u32 i = 0; i < 4; ++i)
        {
            for (u32 j = 0; j < 4; ++j)
            {
                res.elements[i][j] = m.elements[i][j] / s;
            }
        }
    }
    return res;
}

INLINE mat4 mat4_add(mat4 l, mat4 r)
{
    mat4 res;
    for (u32 i = 0; i < 4; ++i)
    {
        for (u32 j = 0; j < 4; ++j)
        {
            res.elements[i][j] = (f32)l.elements[i][j] + (f32)r.elements[i][j];
        }
    }
    return res;
}


INLINE mat4 mat4_sub(mat4 l, mat4 r)
{
    mat4 res;
    for (u32 i = 0; i < 4; ++i)
    {
        for (u32 j = 0; j < 4; ++j)
        {
            res.elements[i][j] = l.elements[i][j] - r.elements[i][j];
        }
    }
    return res;
}

//r is done first and then l
INLINE mat4 mat4_mul(mat4 l, mat4 r)
{
    mat4 res;
    for (u32 col = 0; col < 4; ++col)
    {
        for (u32 row = 0; row < 4; ++row)
        {
            f32 sum = 0;
            for (u32 current_index = 0; current_index < 4; ++current_index)
            {
                sum += (f32)l.elements[current_index][row] * (f32)r.elements[col][current_index];
            }
            res.elements[col][row] = sum;
        }
    }
    return res;
}


INLINE mat4 mat4_translate(vec3 t) //TODO(ilias): check handedness
{
    mat4 res = m4d(1.0f);
    res.elements[3][0] = t.x;
    res.elements[3][1] = t.y;
    res.elements[3][2] = t.z;
    return res;
}

INLINE mat4 mat4_rotate(f32 angle, vec3 axis)
{
    mat4 res = m4d(1.0f);

    axis = vec3_normalize(axis);

    float sinA = sin(to_radians(angle));
    float cosA = cos(to_radians(angle));
    float cos_val = 1.0f - cosA;

    res.elements[0][0] = (axis.x * axis.x * cos_val) + cosA;
    res.elements[0][1] = (axis.x * axis.y * cos_val) + (axis.z * sinA);
    res.elements[0][2] = (axis.x * axis.z * cos_val) - (axis.y * sinA);

    res.elements[1][0] = (axis.y * axis.x * cos_val) - (axis.z * sinA);
    res.elements[1][1] = (axis.y * axis.y * cos_val) + cosA;
    res.elements[1][2] = (axis.y * axis.z * cos_val) + (axis.x * sinA);

    res.elements[2][0] = (axis.z * axis.x * cos_val) + (axis.y * sinA);
    res.elements[2][1] = (axis.z * axis.y * cos_val) - (axis.x * sinA);
    res.elements[2][2] = (axis.z * axis.z * cos_val) + cosA;

    return (res);
}

INLINE mat4 mat4_scale(vec3 s)
{
    mat4 res = m4d(1.f);
    res.elements[0][0] *= s.x;
    res.elements[1][1] *= s.y;
    res.elements[2][2] *= s.z;
    return res;
}

INLINE mat4 mat4_inv(mat4 m)
{
    mat4 res;
    f32 det;
    mat4 inv, inv_out;
    i32 i;

    inv.raw[0] = m.raw[5]  * m.raw[10] * m.raw[15] - 
             m.raw[5]  * m.raw[11] * m.raw[14] - 
             m.raw[9]  * m.raw[6]  * m.raw[15] + 
             m.raw[9]  * m.raw[7]  * m.raw[14] +
             m.raw[13] * m.raw[6]  * m.raw[11] - 
             m.raw[13] * m.raw[7]  * m.raw[10];

    inv.raw[4] = -m.raw[4]  * m.raw[10] * m.raw[15] + 
              m.raw[4]  * m.raw[11] * m.raw[14] + 
              m.raw[8]  * m.raw[6]  * m.raw[15] - 
              m.raw[8]  * m.raw[7]  * m.raw[14] - 
              m.raw[12] * m.raw[6]  * m.raw[11] + 
              m.raw[12] * m.raw[7]  * m.raw[10];

    inv.raw[8] = m.raw[4]  * m.raw[9] * m.raw[15] - 
             m.raw[4]  * m.raw[11] * m.raw[13] - 
             m.raw[8]  * m.raw[5] * m.raw[15] + 
             m.raw[8]  * m.raw[7] * m.raw[13] + 
             m.raw[12] * m.raw[5] * m.raw[11] - 
             m.raw[12] * m.raw[7] * m.raw[9];

    inv.raw[12] = -m.raw[4]  * m.raw[9] * m.raw[14] + 
               m.raw[4]  * m.raw[10] * m.raw[13] +
               m.raw[8]  * m.raw[5] * m.raw[14] - 
               m.raw[8]  * m.raw[6] * m.raw[13] - 
               m.raw[12] * m.raw[5] * m.raw[10] + 
               m.raw[12] * m.raw[6] * m.raw[9];

    inv.raw[1] = -m.raw[1]  * m.raw[10] * m.raw[15] + 
              m.raw[1]  * m.raw[11] * m.raw[14] + 
              m.raw[9]  * m.raw[2] * m.raw[15] - 
              m.raw[9]  * m.raw[3] * m.raw[14] - 
              m.raw[13] * m.raw[2] * m.raw[11] + 
              m.raw[13] * m.raw[3] * m.raw[10];

    inv.raw[5] = m.raw[0]  * m.raw[10] * m.raw[15] - 
             m.raw[0]  * m.raw[11] * m.raw[14] - 
             m.raw[8]  * m.raw[2] * m.raw[15] + 
             m.raw[8]  * m.raw[3] * m.raw[14] + 
             m.raw[12] * m.raw[2] * m.raw[11] - 
             m.raw[12] * m.raw[3] * m.raw[10];

    inv.raw[9] = -m.raw[0]  * m.raw[9] * m.raw[15] + 
              m.raw[0]  * m.raw[11] * m.raw[13] + 
              m.raw[8]  * m.raw[1] * m.raw[15] - 
              m.raw[8]  * m.raw[3] * m.raw[13] - 
              m.raw[12] * m.raw[1] * m.raw[11] + 
              m.raw[12] * m.raw[3] * m.raw[9];

    inv.raw[13] = m.raw[0]  * m.raw[9] * m.raw[14] - 
              m.raw[0]  * m.raw[10] * m.raw[13] - 
              m.raw[8]  * m.raw[1] * m.raw[14] + 
              m.raw[8]  * m.raw[2] * m.raw[13] + 
              m.raw[12] * m.raw[1] * m.raw[10] - 
              m.raw[12] * m.raw[2] * m.raw[9];

    inv.raw[2] = m.raw[1]  * m.raw[6] * m.raw[15] - 
             m.raw[1]  * m.raw[7] * m.raw[14] - 
             m.raw[5]  * m.raw[2] * m.raw[15] + 
             m.raw[5]  * m.raw[3] * m.raw[14] + 
             m.raw[13] * m.raw[2] * m.raw[7] - 
             m.raw[13] * m.raw[3] * m.raw[6];

    inv.raw[6] = -m.raw[0]  * m.raw[6] * m.raw[15] + 
              m.raw[0]  * m.raw[7] * m.raw[14] + 
              m.raw[4]  * m.raw[2] * m.raw[15] - 
              m.raw[4]  * m.raw[3] * m.raw[14] - 
              m.raw[12] * m.raw[2] * m.raw[7] + 
              m.raw[12] * m.raw[3] * m.raw[6];

    inv.raw[10] = m.raw[0]  * m.raw[5] * m.raw[15] - 
              m.raw[0]  * m.raw[7] * m.raw[13] - 
              m.raw[4]  * m.raw[1] * m.raw[15] + 
              m.raw[4]  * m.raw[3] * m.raw[13] + 
              m.raw[12] * m.raw[1] * m.raw[7] - 
              m.raw[12] * m.raw[3] * m.raw[5];

    inv.raw[14] = -m.raw[0]  * m.raw[5] * m.raw[14] + 
               m.raw[0]  * m.raw[6] * m.raw[13] + 
               m.raw[4]  * m.raw[1] * m.raw[14] - 
               m.raw[4]  * m.raw[2] * m.raw[13] - 
               m.raw[12] * m.raw[1] * m.raw[6] + 
               m.raw[12] * m.raw[2] * m.raw[5];

    inv.raw[3] = -m.raw[1] * m.raw[6] * m.raw[11] + 
              m.raw[1] * m.raw[7] * m.raw[10] + 
              m.raw[5] * m.raw[2] * m.raw[11] - 
              m.raw[5] * m.raw[3] * m.raw[10] - 
              m.raw[9] * m.raw[2] * m.raw[7] + 
              m.raw[9] * m.raw[3] * m.raw[6];

    inv.raw[7] = m.raw[0] * m.raw[6] * m.raw[11] - 
             m.raw[0] * m.raw[7] * m.raw[10] - 
             m.raw[4] * m.raw[2] * m.raw[11] + 
             m.raw[4] * m.raw[3] * m.raw[10] + 
             m.raw[8] * m.raw[2] * m.raw[7] - 
             m.raw[8] * m.raw[3] * m.raw[6];

    inv.raw[11] = -m.raw[0] * m.raw[5] * m.raw[11] + 
               m.raw[0] * m.raw[7] * m.raw[9] + 
               m.raw[4] * m.raw[1] * m.raw[11] - 
               m.raw[4] * m.raw[3] * m.raw[9] - 
               m.raw[8] * m.raw[1] * m.raw[7] + 
               m.raw[8] * m.raw[3] * m.raw[5];

    inv.raw[15] = m.raw[0] * m.raw[5] * m.raw[10] - 
              m.raw[0] * m.raw[6] * m.raw[9] - 
              m.raw[4] * m.raw[1] * m.raw[10] + 
              m.raw[4] * m.raw[2] * m.raw[9] + 
              m.raw[8] * m.raw[1] * m.raw[6] - 
              m.raw[8] * m.raw[2] * m.raw[5];

    det = m.raw[0] * inv.raw[0] + m.raw[1] * inv.raw[4] + 
        m.raw[2] * inv.raw[8] + m.raw[3] * inv.raw[12];

    if (det == 0) //in case the matrix is non-invertible
        return m4d(0.f); 

    det = 1.f / det;

    for (i = 0; i < 16; ++i)
        inv_out.raw[i] = inv.raw[i] * det;

    return inv_out;
}

INLINE mat4 orthographic_proj(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
    mat4 res = m4();

    //the quotents are in reverse because we were supposed to do one more matrix multiplication to negate z..
    //its basically two steps in one..
    res.elements[0][0] = 2.0f / (r - l);
    res.elements[1][1] = 2.0f / (t - b);
    res.elements[2][2] = 2.0f / (n - f);
    res.elements[3][3] = 1.0f;

    res.elements[3][0] = (l + r) / (l - r);
    res.elements[3][1] = (b + t) / (b - t);
    res.elements[3][2] = (f + n) / (n - f);

    return res;
}

INLINE mat4 perspective_proj(f32 fov, f32 aspect, f32 n, f32 f)
{
    mat4 res = m4();

    f32 cot = 1.0f / tanf(fov * (PI / 360.0f));

    res.elements[0][0] = cot / aspect;
    res.elements[1][1] = cot;
    res.elements[2][3] = -1.0f;

    res.elements[2][2] = (n + f)/(n - f);

    res.elements[3][2] = (2.f * n * f) / (n - f);
    res.elements[3][3] = 0.0f;

    return res;
}

INLINE mat4 look_at(vec3 eye, vec3 center, vec3 fake_up)
{
    mat4 res = m4();

    vec3 f = vec3_normalize(vec3_sub(center, eye));
    vec3 r = vec3_normalize(vec3_cross(f, fake_up));
    vec3 up = vec3_cross(r, f);

    res.elements[0][0] = r.x;
    res.elements[0][1] = up.x;
    res.elements[0][2] = -f.x;
    res.elements[0][3] = 0.0f;

    res.elements[1][0] = r.y;
    res.elements[1][1] = up.y;
    res.elements[1][2] = -f.y;
    res.elements[1][3] = 0.0f;

    res.elements[2][0] = r.z;
    res.elements[2][1] = up.z;
    res.elements[2][2] = -f.z;
    res.elements[2][3] = 0.0f;

    res.elements[3][0] = -vec3_dot(r, eye);
    res.elements[3][1] = -vec3_dot(up, eye);
    res.elements[3][2] = vec3_dot(f, eye);
    res.elements[3][3] = 1.0f;

    return res;
}

INLINE mat4 
swap_cols_mat4(mat4 mat, u32 col1, u32 col2)
{
    vec4 second_column = v4(mat.elements[col2][0],mat.elements[col2][1], mat.elements[col2][2], mat.elements[col2][3]);
    for (u32 i = 0; i < 4; ++i)
        mat.elements[col2][i] = mat.elements[col1][i];
    for (u32 i = 0; i < 4; ++i)
        mat.elements[col1][i] = second_column.elements[i];
    return mat;
}

INLINE mat4
swap_rows_mat4(mat4 mat, u32 row1, u32 row2)
{
   vec4 second_row = v4(mat.elements[0][row2], mat.elements[1][row2], mat.elements[2][row2], mat.elements[3][row2]); 
   for (u32 i = 0; i < 4; ++i)
       mat.elements[i][row2] = mat.elements[i][row1];
   for (u32 i = 0; i < 4; ++i)
       mat.elements[i][row1] = second_row.elements[i];
   return mat;
}

INLINE mat4 negate_row_mat4(mat4 mat, u32 row)
{
    for (u32 i = 0; i < 4; ++i)
        mat.elements[i][row] = -1.f * mat.elements[i][row];
    return mat;
}

INLINE mat4 negate_col_mat4(mat4 mat, u32 col)
{
    for (u32 i = 0; i < 4; ++i)
        mat.elements[col][i] = -1.f * mat.elements[col][i];
    return mat;
}
//we must swap columns 2 and 3 then swap rows 2 and 3
//and then negate column 3 and row 3 TODO(ilias): check this shit
INLINE mat4
blender_to_opengl_mat4(mat4 mat)
{
   mat = swap_cols_mat4(mat, 2,3);
   mat = swap_rows_mat4(mat,2,3);
   mat = negate_col_mat4(mat, 3);
   mat = negate_row_mat4(mat, 3);
   return mat;
}
INLINE mat4
blender_to_opengl(mat4 mat)
{
   return mat;
}



INLINE mat4
maya_to_opengl(mat4 mat)
{
    return mat;
}


//some operator overloading
#ifdef __cplusplus
INLINE vec2 operator+(vec2 l, vec2 r)
{
    vec2 res = vec2_add(l, r);

    return res;
}
INLINE vec2 operator-(vec2 l, vec2 r)
{
    vec2 res = vec2_sub(l, r);

    return res;
}
INLINE vec2 operator*(vec2 l, vec2 r)
{
    vec2 res = vec2_mul(l, r);

    return res;
}
INLINE vec2 operator*(vec2 l, f32 x)
{
    vec2 res = vec2_mulf(l, x);

    return res;
}

INLINE vec2 operator/(vec2 l, vec2 r)
{
    vec2 res = vec2_div(l, r);

    return res;
}

INLINE vec2 operator/(vec2 l,f32 r)
{
    vec2 res = vec2_divf(l, r);

    return res;
}

INLINE vec3 operator+(vec3 l, vec3 r)
{
    vec3 res = vec3_add(l, r);

    return res;
}
INLINE vec3 operator-(vec3 l, vec3 r)
{
    vec3 res = vec3_sub(l, r);

    return res;
}
INLINE vec3 operator*(vec3 l, vec3 r)
{
    vec3 res = vec3_mul(l, r);

    return res;
}
INLINE vec3 operator*(vec3 l, f32 r)
{
    vec3 res = vec3_mulf(l, r);

    return res;
}
INLINE vec3 operator/(vec3 l, vec3 r)
{
    vec3 res = vec3_div(l, r);

    return res;
}
INLINE vec3 operator/(vec3 l,f32 r)
{
    vec3 res = vec3_divf(l, r);

    return res;
}

INLINE vec4 operator+(vec4 l, vec4 r)
{
    vec4 res = vec4_add(l, r);

    return res;
}
INLINE vec4 operator-(vec4 l, vec4 r)
{
    vec4 res = vec4_sub(l, r);

    return res;
}
INLINE vec4 operator*(vec4 l, vec4 r)
{
    vec4 res = vec4_mul(l, r);

    return res;
}
INLINE vec4 operator*(vec4 l, f32 r)
{
    vec4 res = vec4_mulf(l, r);

    return res;
}
INLINE vec4 operator/(vec4 l, vec4 r)
{
    vec4 res = vec4_div(l, r);

    return res;
}
INLINE vec4 operator/(vec4 l,f32 r)
{
    vec4 res = vec4_divf(l, r);

    return res;
}


INLINE mat4 operator+(mat4 l, mat4 r)
{
    mat4 res = mat4_add(l,r);

    return res;
}
INLINE mat4 operator-(mat4 l, mat4 r)
{
    mat4 res = mat4_sub(l,r);

    return res;
}

INLINE mat4 operator*(mat4 l, mat4 r)
{
    mat4 res = mat4_mul(l,r);

    return res;
}

INLINE mat4 operator*(mat4 l,f32 r)
{
    mat4 res = mat4_mulf(l,r);

    return res;
}

#endif

//QUATERNION LIB 
typedef union Quaternion
{
    struct
    {
        union
        {
            vec3 xyz;
            struct
            {
                f32 x,y,z;
            };
        };
        f32 w;
    };
    f32 elements[4];

}Quaternion;

INLINE Quaternion quat(f32 x, f32 y, f32 z, f32 w)
{
    Quaternion res;

    res.x = x;
    res.y = y;
    res.z = z;
    res.w = w;
    //res = {x,y,z,w};

    return res;
}

INLINE Quaternion quat_vec4(vec4 vec)
{
    Quaternion res;

    res.x = vec.x;
    res.y = vec.y;
    res.z = vec.z;
    res.w = vec.w;

    return res;
}

INLINE Quaternion quat_add(Quaternion l, Quaternion r)
{
    Quaternion res;

    res.x = l.x + r.x;
    res.y = l.y + r.y;
    res.z = l.z + r.z;
    res.w = l.w + r.w;

    return res;
}

INLINE Quaternion quat_sub(Quaternion l, Quaternion r)
{
    Quaternion res;

    res.x = l.x - r.x;
    res.y = l.y - r.y;
    res.z = l.z - r.z;
    res.w = l.w - r.w;

    return res;
}

//TODO(ilias): check the scalars..
INLINE Quaternion quat_mul(Quaternion l, Quaternion r)
{
    Quaternion res;

    res.w = (l.w * r.w) - (l.x * r.x) - (l.y * r.y) - (l.z * r.z);
    res.x = (l.w * r.w) + (l.x * r.w) + (l.y * r.z) - (l.z * r.y);
    res.y = (l.w * r.y) - (l.x * r.z) + (l.y * r.w) + (l.z * r.x);
    res.z = (l.w * r.z) - (l.x * r.y) - (l.y * r.x) + (l.z * r.w);
    
    return res;
}


INLINE Quaternion quat_mulf(Quaternion l, f32 val)
{
    Quaternion res;

    res.x = l.x * val;
    res.y = l.y * val;
    res.z = l.z * val;
    res.w = l.w * val;

    return res;
}

INLINE Quaternion quat_divf(Quaternion l, f32 val)
{
    assert(val);
    Quaternion res;
    
    res.x = l.x / val;
    res.y = l.y / val;
    res.z = l.z / val;
    res.w = l.w / val;

    return res;
}

INLINE f32 quat_dot(Quaternion l, Quaternion r)
{
   f32 res;

   res = (l.x * r.x) + (l.y * r.y) + (l.z * r.z) + (l.w * r.w);

   return res;
}

INLINE b32 quat_equals(Quaternion l, Quaternion r)
{
    f32 dot = quat_dot(l,r);
    return 1 ? 0 : fabs(dot - 1.f) < 0.001f;
}


INLINE Quaternion quat_inv(Quaternion l)
{
    Quaternion res;

    f32 len = sqrtf(dot_quat(l,l));
    res = quat_divf(l, len);

    return res;
}

INLINE Quaternion slerp(Quaternion l, Quaternion r, f32 time)
{
    Quaternion res;

    //some complex shit

    return res;
}

INLINE Quaternion quat_from_angle(vec3 axis, f32 angle)
{
    Quaternion res;

    vec3 axis_normalized = vec3_normalize(axis);
    //this because quaternions are (i)q(i^-1) so angles are double
    f32 sintheta = sin(angle / 2.f); 

    res.xyz = vec3_mulf(axis_normalized, sintheta);
    res.w = cos(angle / 2.f);
    
    return res;
}

INLINE vec3 quat_to_angle(Quaternion quat)
{
    f32 theta = acos(quat.w) *2.f;

    float ax = quat.x / sin(acos(theta));
    float ay = quat.y / sin(acos(theta));
    float az = quat.z / sin(acos(theta));

    return v3(ax,ay,az);
}



INLINE Quaternion quat_normalize(Quaternion l)
{
    Quaternion res;

    f32 len = sqrtf(quat_dot(l,l));
    res = quat_divf(l,len);

    return res;
}

INLINE Quaternion nlerp(Quaternion l, Quaternion r, f32 time)
{
    Quaternion res;

    //we gotta interpolate all quaternion components
    res.x = lerp(l.x, r.x, time);
    res.y = lerp(l.y, r.y, time);
    res.z = lerp(l.z, r.z, time);
    res.w = lerp(l.w, r.w, time);

    res = quat_normalize(res);
    
    return res;
}

//taken from HMMATH.. investigate further..
INLINE mat4 quat_to_mat4(Quaternion l)
{
    mat4 res;

    Quaternion norm_quat = quat_normalize(l);

    f32 XX, YY, ZZ, XY, XZ, YZ, WX, WY, WZ;

    XX = norm_quat.x * norm_quat.x;
    YY = norm_quat.y * norm_quat.y;
    ZZ = norm_quat.z * norm_quat.z;
    XY = norm_quat.x * norm_quat.y;
    XZ = norm_quat.x * norm_quat.z;
    YZ = norm_quat.y * norm_quat.z;
    WX = norm_quat.w * norm_quat.x;
    WY = norm_quat.w * norm_quat.y;
    WZ = norm_quat.w * norm_quat.z;

    res.elements[0][0] = 1.0f - 2.0f * (YY + ZZ);
    res.elements[0][1] = 2.0f * (XY + WZ);
    res.elements[0][2] = 2.0f * (XZ - WY);
    res.elements[0][3] = 0.0f;

    res.elements[1][0] = 2.0f * (XY - WZ);
    res.elements[1][1] = 1.0f - 2.0f * (XX + ZZ);
    res.elements[1][2] = 2.0f * (YZ + WX);
    res.elements[1][3] = 0.0f;

    res.elements[2][0] = 2.0f * (XZ + WY);
    res.elements[2][1] = 2.0f * (YZ - WX);
    res.elements[2][2] = 1.0f - 2.0f * (XX + YY);
    res.elements[2][3] = 0.0f;

    res.elements[3][0] = 0.0f;
    res.elements[3][1] = 0.0f;
    res.elements[3][2] = 0.0f;
    res.elements[3][3] = 1.0f;

    return res;
}

//taken directly from HandmadeMath.. investigate its authenticity 
INLINE Quaternion 
mat4_to_quat(mat4 m)
{
    float T;
    Quaternion Q;

    if (m.elements[2][2] < 0.0f) {
        if (m.elements[0][0] > m.elements[1][1]) {
            T = 1 + m.elements[0][0] - m.elements[1][1] - m.elements[2][2];
            Q = quat(
                T,
                m.elements[0][1] + m.elements[1][0],
                m.elements[2][0] + m.elements[0][2],
                m.elements[1][2] - m.elements[2][1]
            );
        } else {
            T = 1 - m.elements[0][0] + m.elements[1][1] - m.elements[2][2];
            Q = quat(
                m.elements[0][1] + m.elements[1][0],
                T,
                m.elements[1][2] + m.elements[2][1],
                m.elements[2][0] - m.elements[0][2]
            );
        }
    } else {
        if (m.elements[0][0] < -m.elements[1][1]) {

            T = 1 - m.elements[0][0] - m.elements[1][1] + m.elements[2][2];
            Q = quat(
                m.elements[2][0] + m.elements[0][2],
                m.elements[1][2] + m.elements[2][1],
                T,
                m.elements[0][1] - m.elements[1][0]
            );
        } else {
            T = 1 + m.elements[0][0] + m.elements[1][1] + m.elements[2][2];
            Q = quat(
                m.elements[1][2] - m.elements[2][1],
                m.elements[2][0] - m.elements[0][2],
                m.elements[0][1] - m.elements[1][0],
                T
            );
        }
    }

    Q = quat_mulf(Q, 0.5f / sqrtf(T));

    return Q;
}


//TGA LIB 
enum {
    TGA_ERROR_FILE_OPEN = 1,
    TGA_ERROR_READING_FILE,
    TGA_ERROR_INDEXED_COLOR, 
    TGA_ERROR_MEMORY,
    TGA_ERROR_COMPRESSED_FILE, 
    TGA_OK
};

typedef struct TGAInfo
{
    i32 status;
    u8 type, bits_per_pixel;
    i16 width, height;
    u8 *image_data;
}TGAInfo;

internal TGAInfo* tga_init_image_RGB(i16 width, i16 height)
{
    TGAInfo* info;
    info = (TGAInfo*)ALLOC(sizeof(TGAInfo));
    info->width = width;
    info->height = height;
    info->bits_per_pixel = 24;
    info->type = 2; 
    info->status = TGA_OK;
    info->image_data = (u8*)ALLOC(sizeof(u8) * width * height * (info->bits_per_pixel / 8));
    return info;
}

internal void 
tga_load_header(FILE *file, TGAInfo *info) {

	u8 c_garbage;
	i16 i_garbage;

	fread(&c_garbage, sizeof(u8), 1, file);
	fread(&c_garbage, sizeof(u8), 1, file);

    // type must be 2 or 3
	fread(&info->type, sizeof(u8), 1, file);

	fread(&i_garbage, sizeof(i16), 1, file);
	fread(&i_garbage, sizeof(i16), 1, file);
	fread(&c_garbage, sizeof(u8), 1, file);
	fread(&i_garbage, sizeof(i16), 1, file);
	fread(&i_garbage, sizeof(i16), 1, file);

	fread(&info->width, sizeof(i16), 1, file);
	fread(&info->height, sizeof(i16), 1, file);
	fread(&info->bits_per_pixel, sizeof(u8), 1, file);

	fread(&c_garbage, sizeof(u8), 1, file);
}
internal void tga_load_image_data(FILE *file, TGAInfo *info) {

	i32 mode,total,i;
	u8 aux;

    // mode equal the number of components for each pixel
	mode = info->bits_per_pixel / 8;
    // total is the number of bytes we'll have to read
	total = info->height * info->width * mode;
	
	fread(info->image_data,sizeof(u8),total,file);

    // mode=3 or 4 implies that the image is RGB(A). However TGA
    // stores it as BGR(A) so we'll have to swap R and B.
	if (mode >= 3)
		for (i=0; i < total; i+= mode) {
			aux = info->image_data[i];
			info->image_data[i] = info->image_data[i+2];
			info->image_data[i+2] = aux;
		}
    //flip the image
    u8* pixels_new = (u8*)ALLOC(sizeof(u8) * info->width * info->height * (info->bits_per_pixel / 8));
    i32 new_i = 0;
    if (mode == 3)
    {
      for (i32 j = info->height - 1; j >=0; --j)
      {
          for (i32 i = 0; i < info->width-1; ++i)
          {
              i32 index = info->width * 3 *j + 3 * i;
              u8 cmp[3];
              pixels_new[new_i++]= (u8)(info->image_data[index]);
              pixels_new[new_i++]= (u8)(info->image_data[index+1]);
              pixels_new[new_i++]= (u8)(info->image_data[index+2]);
        }
      }

    info->image_data = pixels_new;
    }else if (mode == 4)
    {
     for (i32 j = info->height - 1; j >=0; --j)
      {
          for (i32 i = 0; i < info->width; ++i)
          {
              i32 index = info->width * 4 *j + 4 * i;
              u8 cmp[3];
              pixels_new[new_i++]= (u8)(info->image_data[index]);
              pixels_new[new_i++]= (u8)(info->image_data[index+1]);
              pixels_new[new_i++]= (u8)(info->image_data[index+2]);
              pixels_new[new_i++]= (u8)(info->image_data[index+3]);
        }
      }
     free(info->image_data);
    info->image_data = pixels_new;
    }
}

internal TGAInfo* 
tga_load(char *filename)
{
    TGAInfo *info;
    FILE* file;
    i32 mode,total;

    //allocate memory for TGAInfo
    info = (TGAInfo*)ALLOC(sizeof(TGAInfo));
    if(info == NULL)return(NULL);

    //open file for binary reading
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        info->status = TGA_ERROR_FILE_OPEN;
        //fclose(file);
        return info;
    }

    //we load the header and fill out neccesary info
    tga_load_header(file, info);

    //check if color indexed
    if (info->type == 1)
    {
        info->status = TGA_ERROR_INDEXED_COLOR;
        fclose(file);
        return info;
    }

    //check if compressed
    if ((info->type != 2) && (info->type != 3))
    {
       info->status = TGA_ERROR_COMPRESSED_FILE; 
    }

    mode = info->bits_per_pixel / 8;
    total =info->height * info->width * mode;
    info->image_data = (u8*)ALLOC(total * sizeof(u8));

    //check if memory is ok
    if (info->image_data == NULL)
    {
        info->status = TGA_ERROR_MEMORY;
        fclose(file);
        return info;
    }

    //load the fucking image
    tga_load_image_data(file, info);
    if (ferror(file))
    {
        info->status = TGA_ERROR_READING_FILE;
        fclose(file);
        info->status = TGA_OK;
        return info;
    }
    fclose(file);
    info->status = TGA_OK;

    return info;
}


internal i16 
tga_save(char *filename, i16 width, i16 height, u8 bits_per_pixel, u8 *image_data)
{
    u8 c_garbage = 0, type,mode, aux;
    i16 i_garbage;
    i32 i;
    FILE* file;
    file = fopen(filename, "wb");
    if (file == NULL)
        return TGA_ERROR_FILE_OPEN;
    mode = bits_per_pixel / 8;
    if ((bits_per_pixel == 24) || (bits_per_pixel == 32))
        type = 2;
    else
        type = 3;

    // write the header
	fwrite(&c_garbage, sizeof(u8), 1, file);
	fwrite(&c_garbage, sizeof(u8), 1, file);

	fwrite(&type, sizeof(u8), 1, file);

	fwrite(&i_garbage, sizeof(i16), 1, file);
	fwrite(&i_garbage, sizeof(i16), 1, file);
	fwrite(&c_garbage, sizeof(u8), 1, file);
	fwrite(&i_garbage, sizeof(i16), 1, file);
	fwrite(&i_garbage, sizeof(i16), 1, file);

	fwrite(&width, sizeof(i16), 1, file);
	fwrite(&height, sizeof(i16), 1, file);
	fwrite(&bits_per_pixel, sizeof(u8), 1, file);

	fwrite(&c_garbage, sizeof(u8), 1, file);


    // convert the image data from RGB(a) to BGR(A)
	if (mode >= 3)
        for (i=0; i < width * height * mode ; i+= mode) {
            aux = image_data[i];
            image_data[i] = image_data[i+2];
            image_data[i+2] = aux;
        }

    // save the image data
	fwrite(image_data, sizeof(u8), width * height * mode, file);
	fclose(file);
	//free(image_data);
    //image_data = NULL;

	return TGA_OK;
}

internal void
tga_destroy(TGAInfo * info)
{
    if (info != NULL)
    {
        if (info->image_data != NULL)
            free(info->image_data);
        free(info);
        info = NULL;
    }
    
}

//MEMORY STUFF

typedef struct Arena
{
    void *memory;
    u32 memory_size;
    u32 current_offset;
} Arena;


internal Arena 
arena_init(void* memory, u32 size)
{
    Arena a = {0};
    a.memory = memory;
    a.memory_size = size;
    a.current_offset = 0;

    return a;
}

internal void
arena_free(Arena* arena, u32 size)
{
    //do nothing
}

internal void * 
arena_alloc(Arena* arena, u32 size)
{
    void* mem = 0;

    if (arena->current_offset + size <= arena->memory_size)
    {
        //position of next available byte
        mem = (void *)((u8*)arena->memory + arena->current_offset); 
        arena->current_offset += size;
    }
    //we return a pointer to size free bytes of memory
    return mem;
}

internal void
arena_clear(Arena* arena)
{
    arena->current_offset = 0;
}

internal void 
arena_zero(Arena* arena)
{
    memset(arena->memory, 0, arena->memory_size);
}

//STRING STUFF

typedef struct String
{
    char* data;
    u32 len;
    u32 size;
    u32 max_size;
    b32 is_constant;
}String;

internal String
init_string_in_arena(Arena* arena, u32 size)
{
    String str = {0};

    str.data = (char*)arena_alloc(arena, size);
    if (str.data)
    {
        str.len = size - 1;
        str.size = size;
        str.max_size = size;
        str.is_constant = 0;

        str.data[size-1] = '\0';
    }
    return str;
}

internal String str(Arena* arena, char* characters)
{

    String s = init_string_in_arena(arena, str_size(characters) + 1);
    memcpy(s.data, characters, str_size(characters) + 1);
    return s; 
}

internal String substr(Arena* arena, char* characters, i32 start, i32 finish)
{
    assert(finish - start < str_size(characters));
    assert(start < str_size(characters));
    String s = init_string_in_arena(arena, finish - start + 1);
    memcpy(s.data , characters + start, finish - start);
    return s; 
}



//A stretchy buffer implementation [C99]

//HOW TO USE: declare your array of prefered type as TYPE *arr = NULL;
//then every time you need to insert something, buf_push(arr, ELEMENT);
//if you want to access a certain element you arr[i];
//that's all, also you can buf_free(arr);

#ifdef __cplusplus
extern "C" {
#endif
typedef struct BufHdr
{
    u32 len;
    u32 cap;
    char buf[0];
}BufHdr;



#define buf__hdr(b) ((BufHdr*)((char*)b - offsetof(BufHdr, buf)))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_end(b) ((b) + buf_len(b))

#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : (*((void**)&(b)) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_push(b, ...) (buf_fit((b), 1 + buf_len(b)), (b)[buf__hdr(b)->len++] = (__VA_ARGS__))
#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)

internal void *buf__grow(const void *buf, u32 new_len, u32 element_size)
{
   u32 new_cap = max(16, max(1 + 2*buf_cap(buf), new_len));
   assert(new_len <= new_cap);
   u32 new_size = offsetof(BufHdr, buf) + new_cap * element_size;
   BufHdr *new_hdr; 
   if(buf) 
   { 
       new_hdr = (BufHdr*)REALLOC(buf__hdr(buf), new_size); 
   }
   else
   { 
       new_hdr = (BufHdr*)ALLOC(new_size);
       new_hdr->len = 0;
   }
   new_hdr->cap = new_cap;
   return new_hdr->buf;// + offsetof(BufHdr, buf);
}
/* example usage of stretchy buffer
{
        int *arr = NULL;
        buf_push(arr, 1);
        buf_push(arr, 2);
        buf_push(arr, 3);
        buf_push(arr, 4);
        buf_push(arr, 5);
        buf_push(arr, 6);

        for (int i = 0; i < 6; ++i)
        {
            int x = arr[i];
            assert(x == i+1);
        }

        buf_free(arr);
}
*/


//A LINKED-LIST INT-TO-INT HASHMAP IMPLEMENTATION

typedef struct IntPair
{
   i32 key;
   i32 value;
   struct IntPair *next;
}IntPair;

typedef struct IntHashMap
{
    IntPair **data;
    u32 size;
}IntHashMap;

internal IntHashMap 
hashmap_create(u32 size)
{
    IntHashMap res;
    res.size = size;
    //res.data = (IntPair**)arena_alloc(&global_platform.permanent_storage, sizeof(IntPair*) * size);
    res.data = (IntPair**)ALLOC(sizeof(IntPair*) * size);
    i32 i;
    for (i = 0; i < size; ++i)
        res.data[i] = NULL;
    return res;
}
internal i32
hash_code(IntHashMap* table, i32 key)
{
    return abs((i32)(key % table->size));
}

internal void
hashmap_insert(IntHashMap* table, i32 key, i32 val)
{
   i32 pos = hash_code(table, key);
   IntPair *list_to_insert_pair = table->data[pos];
   IntPair *new_pair = (IntPair*)ALLOC(sizeof(IntPair));
   IntPair *iter = &list_to_insert_pair[0];
   while (iter)
   {
        if (iter->key == key)
        {
            iter->value = val;
            return;
        }
        iter = iter->next;
   }
   new_pair->key = key;
   new_pair->value = val;
   new_pair->next = &list_to_insert_pair[0];
   table->data[pos] = new_pair;
}

internal i32 
hashmap_lookup(IntHashMap* table, u32 key)
{
    u32 pos = hash_code(table, key);
    IntPair* list_to_search = table->data[pos];
    IntPair* iter = list_to_search;
    while (iter)
    {
       if (iter->key == key)
       {
            return iter->value;
       }
       iter = iter->next;
    }
    return -1;
}

static u32 
hashmap_remove(IntHashMap *table, u32 key)
{
    u32 pos = hash_code(table, key);
    IntPair *list_to_search = table->data[pos];
    IntPair *iter = list_to_search;
    IntPair *prev = NULL;
    while (iter)
    {
       if (iter->key == key)
       {
          if (prev == NULL)
          {
              table->data[pos] = iter->next;
              free(iter);
              return 1;
          }else
          {
            prev->next = iter->next;
            free(iter);
            return 1;
          }
       }
       prev = iter;
       iter = iter->next;
    }
    return 0;
}

internal void *free_next(IntPair *p)
{
    free_next(p->next);
    free(p);
}

internal void hashmap_destroy(IntHashMap *table)
{
    for (u32 i = 0; i < table->size; ++i)
    {
        IntPair *next = table->data[i];
        free_next(next);
    }
}








#ifdef __cplusplus
}
#endif


#endif










