## A Game Engine from scratch!

#### Planned Engine Features

##### - 3D Models
  - [ ]OBJ/Collada loading supported
  - [ ]Multitexturing will be supported
  - [ ]Custom materials will be supported via .mtl files
  
##### - Shaders
  - [ ]Shaders loaded with architecture specific IO 
  - [x]Runtime Shader Compilation
  - [ ]Maybe have them all being queriable 
      inside a huge shader array (maybe inside renderer?)
  - [x]set_type functions for all kinds of uniform types

##### - Textures
  - [x]All textures will be cast to RGBA for ease of use
  - [x]TGA format will be supported fully (RGB and RGBA w/RLE compression)
  - [ ]Any other format will be loaded via stb_image.h

##### - Sprites
  - [ ]Have animated sprites renderable through the Renderer
  - [ ]Animated throught textures

##### - Math Lib
  - [x]Vectors (vec3/vec4)
  - [x]Matrices (mat4)
  - [x]Quaternions (Quat)
  - [x]Camera / Projection stuff (look_at, persp, ortho)
  - [ ]Random number generator! (non std lib)

##### - Data Structures
  - [ ]Dynamics Arrays
  - [ ]Hashmaps
  - [x]Engine Specific Allocators

##### - Renderer
  - [ ]Be able to draw all kinds of screen rects
  - [ ]Have many framebuffers to draw
  - [ ]RenderGroups and stuff like that (HMH)
  - [ ]Rendering 3D OBJ Models (also 3D quads as 3D models)
  - [ ]Rendering Skeletal Animated Models (Collada)
  - [ ]Rendering Particles! (a thing on its own maybe)
  - [ ]Rendering Text! (a thing on its own maybe)
  - [ ]Batch Rendering for sprites
  - [ ]Point/Directional Lights (maybe have a manager)
  - [ ]Depth Peeling would be noice
  - [ ]Shadow Mapping of course!
  - [ ]Screen Space Effects

##### - Entity Component System
  - [ ]Entity = id
  - [ ]ComponentManagers for each component with Hashmap and stuff

##### - Animation System
  - [ ]Only Collada files supported
  - [ ]Interpolating between frames
  - [ ]Blending between animations

##### - UI
  - [ ]Implemented through engine's draw_rect and draw_text
  - [ ]has an FBO of its own
  - [ ]ImGui-architecture style

##### - Collision Detection
  - [ ]TBA

##### - Audio System
  - [ ]TBA

##### - Asset System
  - [ ]TBA

##### - Multithreading
  - [ ]TBA

##### - Scenes
  - [ ]TBA

##### - Level Editor
  - [ ]TBA
