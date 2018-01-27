#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <varargs.h>

#include "win32.h"
#include "ray.h"



static offscreen_buffer MainOffscreenBuffer = {};
static world MainWorld = {};
static bool Running = true;
static int WindowWidth = 640;
static int WindowHeight = 480;
static int GlobalSphereCount = 5;
static float CameraDistance = 70.0f;
static camera GlobalCamera = {};





void GenerateWorld(world* World, int SphereCount);
void InitializeCamera(camera* Camera, v3 Position, v3 Direction);



void Win32InitializeBuffer(offscreen_buffer* Buffer, int Width, int Height) {
    
    int BytesPerPixel = 4;
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Width;
    Buffer->Info.bmiHeader.biHeight = Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->Pitch = Width * BytesPerPixel;
    
    Buffer->Buffer = (unsigned char*) malloc(Width * Height * BytesPerPixel);
    
    memset(Buffer->Buffer, 0, Width * Height * BytesPerPixel);
}


void Win32PushBufferToWindow(HDC DeviceContext, offscreen_buffer* Buffer, int WindowWidth, int WindowHeight) {
    StretchDIBits(
        DeviceContext,
        0, 0, Buffer->Width, Buffer->Height,
        0, 0, WindowWidth, WindowHeight,
        Buffer->Buffer,
        &(Buffer->Info),
        DIB_RGB_COLORS,
        SRCCOPY
        );
}


void Win32DebugPrint(char* fmt, ...) {
    char buffer[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    OutputDebugStringA(buffer);
    va_end(args);
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT Result = 0;
    
    switch (uMsg) {
        case WM_DESTROY: 
        case WM_QUIT: {
            Running = false;
            //PostQuitMessage(0);
        } break;
        case WM_KEYDOWN: {
            switch(wParam) {
                case VK_UP: {
                    MainWorld.Planes[0].d -= 1.0f;
                } break;
                case VK_DOWN: {
                    MainWorld.Planes[0].d += 1.0f;
                } break;
                default: {
                    GenerateWorld(&MainWorld, GlobalSphereCount);
                }
            }
            
        } break;
        case WM_LBUTTONDOWN: {
            
            int MouseX = lParam & 0xFFFF;
            int MouseY = WindowHeight - (lParam >> 16);
            Win32DebugPrint("Mouse x: %i, Mouse y: %i\n", MouseX, MouseY);
            
            image_buffer Buffer = {};
            Buffer.Width = WindowWidth;
            Buffer.Height = WindowHeight;
            
            v3 FilmP = BufferCoords2FilmPoint(&Buffer, &GlobalCamera, MouseX, MouseY);
            v3 RayDir = normal(FilmP - GlobalCamera.position);
            
            float HitDistance = INFINITY;
            int HitSphereIndex;
            bool Hit = false;
            
            for(int i = 0; i < MainWorld.SphereCount; i++){
                float t0 = INFINITY;
                float t1 = INFINITY;
                if(IntersectSphere(MainWorld.Spheres[i], GlobalCamera.position, RayDir, &t0, &t1)){
                    Hit = true;
                    if(t0 < 0) t0 = t1;
                    if(t0 < HitDistance) {
                        HitDistance = t0;
                        HitSphereIndex = i;
                    }
                }
            }
            
            if(Hit){
                MainWorld.Spheres[HitSphereIndex].Color = RandomColor();
            }
            
            
        } break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            Win32PushBufferToWindow(hdc, &MainOffscreenBuffer, WindowWidth, WindowHeight);
            EndPaint(hwnd, &ps);
        } break;
        
        default: {
            Result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }
    return Result;	
}




void GenerateWorld(world* World, int SphereCount){
    if (World->Spheres){
        free(World->Spheres);
    }
    
    World->SphereCount = SphereCount;
    World->Spheres = GenerateSpheres(World->SphereCount, 25, 25, 25, 1, 10);
}


void InitializeCamera(camera* Camera, v3 Position, v3 Direction){
    Camera->position = Position;
    Camera->direction = Direction;
}


int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "ray";
    
    RegisterClass(&wc);
    
    DWORD WindowStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
    RECT r = { 0 };
    r.right = WindowWidth;
    r.bottom = WindowHeight;
    AdjustWindowRect(&r, WindowStyle, FALSE);
    
    HWND window = CreateWindowEx(
        0,
        "ray",
        "ray",
        WindowStyle,
        CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
        0,
        0,
        hInstance,
        0
        );
    
    if (window == NULL) {
        return 0;
    }
    
    HDC DeviceContext = GetDC(window);
    ShowWindow(window, nCmdShow);
    Win32InitializeBuffer(&MainOffscreenBuffer, WindowWidth, WindowHeight);
    
    
    //Initialize scene
    MainWorld.SphereCount = GlobalSphereCount;
    MainWorld.Spheres = GenerateSpheres(MainWorld.SphereCount, 25, 25, 25, 1, 10);
    InitializeCamera(&GlobalCamera, v3(0, 0, 100), v3(0, 0, -1));
    
    
    
    plane Plane = {};
    Plane.N = { 0.0f, 1.0f, 0.0f };
    Plane.d = 5;
    Plane.Color = { 0.5f,0.5f,0.5f };
    
    plane Planes[1];
    Planes[0] = Plane;
    
    MainWorld.PlaneCount = 1;
    MainWorld.Planes = Planes;
    
    
    MainWorld.SkyColor = { 0.0f, 0.0f, 0.1f };
    
    
    int Degrees = 270;
    
    
    //timer stuff
    LARGE_INTEGER TicksPerSecond;
    LARGE_INTEGER TicksElapsedLast;
    float SecondsPerTick;
    
    QueryPerformanceFrequency(&TicksPerSecond);
    SecondsPerTick = 1.0 / TicksPerSecond.QuadPart;
    QueryPerformanceCounter(&TicksElapsedLast);
    
    
    
    while (Running) {
        
        
        //Camera manipulation
        if (Degrees >= 360) {
            Degrees = 0;
        }
        
        double Angle = (Degrees / 360.0) * M_PI * 2.0f;
        
        float X = cos(Angle);
        float Z = -sin(Angle);
        
        GlobalCamera.position.x = X * CameraDistance;
        GlobalCamera.position.z = Z * CameraDistance;
        GlobalCamera.direction.x = -X;
        GlobalCamera.direction.z = -Z;
        
        Degrees++;
        
        //push buffer to screen
        image_buffer Buffer = {};
        Buffer.Width = MainOffscreenBuffer.Width;
        Buffer.Height = MainOffscreenBuffer.Height;
        Buffer.Pitch = MainOffscreenBuffer.Pitch;
        Buffer.Buffer = MainOffscreenBuffer.Buffer;
        Render(&Buffer, &MainWorld, &GlobalCamera);
        
        Win32PushBufferToWindow(DeviceContext, &MainOffscreenBuffer, WindowWidth, WindowHeight);
        
        
        
        //timer measurement
        LARGE_INTEGER TicksElapsedNow;
        QueryPerformanceCounter(&TicksElapsedNow);
        int64_t TicksElapsedDifference = TicksElapsedNow.QuadPart - TicksElapsedLast.QuadPart;
        
        float MilisecondsElapsedDifference = TicksElapsedDifference * SecondsPerTick * 1000;
        //Win32DebugPrint("milliseconds this frame: %f\n", MilisecondsElapsedDifference);
        
        TicksElapsedLast = TicksElapsedNow;
        
        
        
        //message loop
        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
    return 0;
}












