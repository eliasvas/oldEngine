#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include "platform.h"
Platform global_platform;
//errors are written here, the program will crash and produce an error
char info_log[512]; 
char error_log[512]; 
global HDC global_device_context;
global HWND WND;
#include <windows.h>
#include "game.c"
#include "win32_opengl.c"
#include "tools.h"
//these are timers!
global LARGE_INTEGER fr,st,ft;



internal LRESULT Win32WindowProc(HWND hWnd, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = {0};
    if (message == WM_SIZE)
        {
           RECT rect;
           GetClientRect(hWnd, &rect);
           //glViewport(0, 0, (GLsizei)rect.right, (GLsizei)rect.bottom); //for some reason this is useless 
           global_platform.window_width = rect.right;
           global_platform.window_height = rect.bottom;
           global_platform.window_resized = TRUE;
    }else if (message == WM_CLOSE || message == WM_DESTROY || message == WM_QUIT){
        global_platform.exit = 1;
    }else if (message == WM_SYSKEYDOWN || message == WM_SYSKEYUP || message == WM_KEYDOWN || message == WM_KEYUP){
        u64 vkey_code = w_param;
        i8 was_down = ((l_param & (1 << 30)) != 0);
        i8 is_down = ((l_param & (1UL << 31)) == 0);

        u64 key_input =0;

        if ((vkey_code >= 'A' && vkey_code <= 'Z') || (vkey_code >='0' && vkey_code <= '9')){
            key_input = (vkey_code >='A' && vkey_code <= 'Z') ? KEY_A + (vkey_code - 'A') : KEY_0 + (vkey_code - '0');
        }else {
            if (vkey_code == VK_F4){
                key_input = KEY_F4;
            }else if (vkey_code == VK_SPACE){
                key_input = KEY_SPACE;
            }else if (vkey_code == VK_MENU)
            {
                key_input= KEY_ALT;
            }else if (vkey_code == VK_TAB)
             {
                 key_input = KEY_TAB;
             }
            else if (vkey_code == VK_LEFT)
            {
                key_input = KEY_LEFT;
            }
            else if (vkey_code == VK_RIGHT)
            {
                key_input = KEY_RIGHT;
            }
            else if (vkey_code == VK_UP)
            {
                key_input = KEY_UP;
            }
            else if (vkey_code == VK_DOWN)
            {
                key_input = KEY_DOWN;
            }
            else if (vkey_code == VK_CONTROL)
            {
                key_input = KEY_CTRL;
            }

           //handle more keys
        }
        if (is_down){
           if (global_platform.key_down[key_input] == 0)
           {
               global_platform.key_pressed[key_input] = 1;
           }
           global_platform.key_down[key_input] = 1;
           global_platform.last_key = (i32)key_input;
           if(global_platform.key_down[KEY_ALT] && key_input == KEY_F4)
           {
               global_platform.exit = 1;
           }
        }else 
        {
            global_platform.key_down[key_input] = 0;
            global_platform.key_pressed[key_input] = 0;
        }
    }else if (message == WM_LBUTTONDOWN)
    {
        global_platform.left_mouse_down = 1;
    }else if (message == WM_LBUTTONUP)
    {
        global_platform.left_mouse_down = 0;
    }else if (message == WM_RBUTTONDOWN)
    {
        global_platform.right_mouse_down = 1;
    }else if (message == WM_RBUTTONUP)
    {
        global_platform.right_mouse_down = 0;
    }

    else{
        result = DefWindowProc(hWnd, message, w_param, l_param);
    }
    return result;
}

internal void
Win32CleanUpOpenGL(HDC *device_context)
{
    wglMakeCurrent(*device_context, 0);
    wglDeleteContext(win32_opengl_context);
}

internal void
Win32OpenGLSetVerticalSync(b32 vsync)
{
    wglSwapIntervalEXT(!!vsync);
}

internal void
Win32OpenGLSwapBuffers(HDC device_context)
{
    wglSwapLayerBuffers(device_context, WGL_SWAP_MAIN_PLANE);
}

//These is the place the program starts from, it makes a window and creates the platform..
//the only thing worry about..
i32 CALLBACK
WinMain(HINSTANCE Instance,

        HINSTANCE PrevInstance,

        LPSTR CommandLine,

        i32 ShowCode)

{
    WNDCLASS windowClass = {0};
    {
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = (WNDPROC)Win32WindowProc;
        windowClass.hInstance = Instance;
        windowClass.lpszClassName = "WindowClass";
        windowClass.hCursor = LoadCursor(0, IDC_ARROW);

    }
    RegisterClass(&windowClass);
    WND = CreateWindow(
            windowClass.lpszClassName, "engine",      // window class, title
            WS_OVERLAPPEDWINDOW, // style
            CW_USEDEFAULT,CW_USEDEFAULT,1280,720,//CW_USEDEFAULT,CW_USEDEFAULT,
            NULL, NULL,                 // parent window, menu
            Instance, NULL);           // instance, param
     
    HDC DC = GetDC(WND);        // Device Context

    //initializing the platform layer
    {
        global_platform.exit = 0;
        global_platform.window_width = 1280;
        global_platform.window_height = 720;
        global_platform.target_fps = 60.f;
        global_platform.current_time = 0.f;
    }

    Win32InitOpenGL(&DC, Instance); 
    //SetWindowText(WND, (LPCSTR)glGetString(GL_VERSION));
    ShowWindow(WND, ShowCode);

    MSG msg;
    QueryPerformanceFrequency(&fr);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND); // <-- this fucker makes artifacts appear in animaiton
    glEnable(GL_MULTISAMPLE); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    //glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);


    //initializing arenas
    {
        u8* permanent_memory = (u8*)malloc(1048576 * 32 * sizeof(u8));
        u8* frame_memory = (u8*)malloc(32768 * 512 * sizeof(u8));
        //mem = (void *)((u8*)arena->memory + arena->current_offset); 
        if (permanent_memory == NULL || frame_memory == NULL)memcpy(error_log, "Not Enough Storage for Arenas", 29);
        global_platform.permanent_storage = arena_init(permanent_memory, 1048576 * 32);
        global_platform.frame_storage = arena_init(frame_memory, 32768 * 256);
     
    }


    init();

    while (!global_platform.exit){
        //zero out previous key presses
        for (u32 key = 0; key < (int)KEY_MAX; ++key)
            global_platform.key_pressed[key] = 0;
        QueryPerformanceCounter(&st);
        while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        {
            POINT mouse;
            GetCursorPos(&mouse);
            ScreenToClient(WND, &mouse);
            global_platform.mouse_x = (f32)(mouse.x);
            global_platform.mouse_y = (f32)(mouse.y);
        }
        
        {
            RECT client_rect;
            GetClientRect(WND, &client_rect);
            global_platform.window_width = client_rect.right - client_rect.left;
            global_platform.window_height = client_rect.bottom - client_rect.top;
        }
        f32 frame_dt = (st.QuadPart - ft.QuadPart)/ (float)fr.QuadPart; //NOTE(ilias): check on actual simulation!!
        update();
        render();
        SwapBuffers(GetDC(WND));
        
        arena_clear(&global_platform.frame_storage);

        QueryPerformanceCounter(&ft);

        i64 frame_count = ft.QuadPart - st.QuadPart;
        i64 desired_frame_count = (f32)fr.QuadPart / global_platform.target_fps;
#if 0
        if (desired_frame_count > frame_count ){
            //NOTE(ilias): wait remaining time --this is wrong
            i64 counts_to_wait = desired_frame_count - frame_count;

            LARGE_INTEGER begin_wait_time_delta;
            LARGE_INTEGER end_wait_time_delta;
            QueryPerformanceCounter(&begin_wait_time_delta);
            //kills the CPU for the delta
            while(counts_to_wait> 0)
            {
                QueryPerformanceCounter(&end_wait_time_delta);
                counts_to_wait -= (end_wait_time_delta.QuadPart - begin_wait_time_delta.QuadPart);
                begin_wait_time_delta = end_wait_time_delta;
            }
            f32 wait_dt = (begin_wait_time_delta.QuadPart - end_wait_time_delta.QuadPart)/ (float)fr.QuadPart; //NOTE(ilias): check on actual simulation!!
            //global_platform.dt = frame_dt + wait_dt;
            //global_platform.current_time += global_platform.dt;//dt;
        }
#endif
           //global_platform.dt = 1.f/60.f;
           global_platform.dt = frame_count / (f32)fr.QuadPart;
           global_platform.current_time += global_platform.dt;//dt;
           global_platform.window_resized = FALSE;

        if (strlen(error_log) != 0){
            MessageBox(WND, error_log, "FATAL ERROR", MB_OK);
            exit(1);
        }

    }
    return 0;
}
