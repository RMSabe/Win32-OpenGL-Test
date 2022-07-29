#ifndef UNICODE
#define UNICODE
#endif

#include <cmath>
#include <thread>
#include <chrono>
#include <windows.h>
#include <GL/gl.h>

#define RUNTIME_STATUS_DONE (-1)
#define RUNTIME_STATUS_DRAW 0

#define __WNDCLASS_NAME "MY_WNDCLASS"

#define __WINDOW_NAME "Win32 Drawing Window"
#define __WINDOW_X_POS 50
#define __WINDOW_Y_POS 50
#define __WINDOW_WIDTH 1000
#define __WINDOW_HEIGHT 500

LPCWSTR WNDCLASS_NAME = TEXT(__WNDCLASS_NAME);
LPCWSTR WINDOW_NAME = TEXT(__WINDOW_NAME);
HINSTANCE hRuntimeInstance;

int runtimeStatus = 0;
HWND hWindow;
HDC hDeviceContext;
HGLRC hRenderingContext;

void delay_us(unsigned long time_us)
{
  std::this_thread::sleep_for(std::chrono::nanoseconds(1000*time_us));
  return;
}

void paintScreen(float red, float green, float blue)
{
  glColor3f(red, green, blue);
  glBegin(GL_QUADS);
  glVertex2f(-1.0f, 1.0f);
  glVertex2f(1.0f, 1.0f);
  glVertex2f(1.0f, -1.0f);
  glVertex2f(-1.0f, -1.0f);
  glEnd();
  return;
}

float x = 0.0f;
float y = 0.0f;
float yi = 0.0f;
float d = 0.0f;
float i = 0.0f;

//The following function draws (roughly) a function, its derivative and its integral.
#define PROC0_X_DOMAIN 12.56f //4 PI == 2 Cycles
#define PROC0_X_STEP 0.01f
#define PROC0_DELAY_TIME_US 400 //Hold 400 us for each drawn step.

void proc_0(void)
{
  if(x > PROC0_X_DOMAIN)
  {
    runtimeStatus = RUNTIME_STATUS_DONE;
    return;
  }
  
  y = sinf(x) + sinf(3*x)/3 + sinf(5*x)/5 + sinf(7*x)/7; //roughly a square wave.
  d = (y - yi)/PROC0_X_STEP;
  i += y*PROC0_X_STEP;
  yi = y;
  
  glBegin(GL_POINTS);
  
  //Function:
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex2f((2*x/PROC0_X_DOMAIN - 1), (0.25f*y));
  
  //Derivative:
  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex2f((2*x/PROC0_X_DOMAIN - 1), (0.25f*d));
  
  //Integral:
  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex2f((2*x/PROC0_X_DOMAIN - 1), (0.25f*i));
  
  glEnd();
  
  delay_us(PROC0_DELAY_TIME_US);
  x += PROC0_X_STEP;
  return;
}

void runtimeProcedure(void)
{
  switch(runtimeStatus)
  {
    case RUNTIME_STATUS_DONE:
      Sleep(10);
      break;
      
    case RUNTIME_STATUS_DRAW:
      proc_0();
      SwapBuffers(hDeviceContext);
      break;
  }
  
  return;
}

void glInit(void)
{
  HDC *pHDC = &hDeviceContext;
  HGLRC *pHRC = &hRenderingContext;
  PIXELFORMATDESCRIPTOR pfd;
  int pixelFormat;
  
  ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
  *pHDC = GetDC(hWindow);
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER);
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.iLayerType = PFD_MAIN_PLANE;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 16;
  pixelFormat = ChoosePixelFormat(hDeviceContext, &pfd);
  SetPixelFormat(hDeviceContext, pixelFormat, &pfd);
  *pHRC = wglCreateContext(hDeviceContext);
  wglMakeCurrent(hDeviceContext, hRenderingContext);
  
  return;
}

void glDeinit(void)
{
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(hRenderingContext);
  ReleaseDC(hWindow, hDeviceContext);
  return;
}

void createMainWindow(void)
{
  DWORD windowStyle = (WS_CAPTION | WS_VISIBLE | WS_SYSMENU | WS_OVERLAPPED);
  RECT rectangle = {
    .left = __WINDOW_X_POS,
    .top = __WINDOW_Y_POS,
    .right = __WINDOW_X_POS + __WINDOW_WIDTH,
    .bottom = __WINDOW_Y_POS + __WINDOW_HEIGHT
  };
  AdjustWindowRect(&rectangle, windowStyle, FALSE);
  
  int xPos = rectangle.left;
  int yPos = rectangle.top;
  int width = rectangle.right - rectangle.left;
  int height = rectangle.bottom - rectangle.top;
  
  hWindow = CreateWindow(WNDCLASS_NAME, WINDOW_NAME, windowStyle, xPos, yPos, width, height, NULL, NULL, hRuntimeInstance, NULL);
  return;
}

LRESULT CALLBACK windowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    case WM_CLOSE:
      PostQuitMessage(0);
      return 0;
  }
  
  return DefWindowProc(hWnd, msg, wParam, lParam);
}

void registerWndClass(void)
{
  WNDCLASS myWndClass;
  ZeroMemory(&myWndClass, sizeof(WNDCLASS));
  myWndClass.hInstance = hRuntimeInstance;
  myWndClass.lpszClassName = WNDCLASS_NAME;
  myWndClass.lpfnWndProc = windowProcedure;
  myWndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  myWndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  RegisterClass(&myWndClass);
  return;
}

void initialize(void)
{
  runtimeStatus = RUNTIME_STATUS_DRAW;
  registerWndClass();
  createMainWindow();
  glInit();
  
  paintScreen(0.0f, 0.0f, 0.0f);
  SwapBuffers(hDeviceContext);
  paintScreen(0.0f, 0.0f, 0.0f);
  SwapBuffers(hDeviceContext);
  
  glColor3f(1.0f, 1.0f, 1.0f);
  ShowWindow(hWindow, SW_SHOW);
  return;
}

void terminate(void)
{
  glDeinit();
  DestroyWindow(hWindow);
  UnregisterClass(WNDCLASS_NAME, hRuntimeInstance);
  return;
}

WINBOOL runtimeContinue(void)
{
  MSG msg;
  while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    if(msg.message == WM_QUIT) return FALSE;
    
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  
  return TRUE;
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
  hRuntimeInstance = hInstance;
  initialize();
  
  while(runtimeContinue()) runtimeProcedure();
  
  terminate();
  return 0;
}
