//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Trace Gen
//
// File: main_win.cpp
//
//-----------------------------------------------------------------------------

// Ran RenderDoc commandline:
// renderdoc cmd convert -f "C:\Users\akharlamov\Documents\NVIDIA Nsight\Captures\triangle.rdc" -o
// "C:\Users\akharlamov\Documents\NVIDIA Nsight\Captures\tri.cpp"

// Defines the entry point that initializes and runs the serialized frame
// capture on Windows
#if _WIN32

#include <Windows.h>

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "gen_main.h"

//-----------------------------------------------------------------------------
// Global Variable for Frame Replay
//-----------------------------------------------------------------------------
int frameLoops = -1;
double accumTimeWithReset = 0;
double accumTime = 0;
double avgTimeWithReset = 0;
double avgTime = 0;
double avgFPSWithReset = 0;
double avgFPS = 0;
uint64_t frames = 0;
double avgFrameMilliseconds = 0;
LARGE_INTEGER performanceCounterFrequency;
bool automated = false;
bool resourceReset = false;
HINSTANCE appInstance;
HWND appHwnd;

#define RDOC_WINDOW_CLASS_NAME L"RenderDoc Frame Loop"
#define RDOC_WINDOW_TITLE L"RenderDoc Frame Loop"

static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL RegisterWndClass(HINSTANCE hInstance, UINT style);
HWND CreateWnd(HINSTANCE hInstance, HINSTANCE hPrevInstance, uint32_t PosX, uint32_t PosY,
               uint32_t Width, uint32_t Height, DWORD Style, DWORD ExtendedStyle);
void CreateResources();
void ReleaseResources();
void Render();

std::string PostStageProgress(const char *stage, uint32_t i, uint32_t N) {
  SetWindowTextA(appHwnd, StageProgressString(stage, i, N).c_str());
}

//-----------------------------------------------------------------------------
// MainWndProc
//-----------------------------------------------------------------------------
static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
    case WM_KEYDOWN:
      if(wParam == VK_ESCAPE)
      {
        PostQuitMessage(0);
        return 0;
      }

      break;
    case WM_CLOSE:
      PostQuitMessage(0);
      return 0;
      break;
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// RegisterWndClass
//-----------------------------------------------------------------------------
BOOL RegisterWndClass(HINSTANCE hInstance, UINT style)
{
  // Populate the struct
  WNDCLASS wc;
  wc.style = style;
  wc.lpfnWndProc = (WNDPROC)MainWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon((HINSTANCE)NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName = L"";
  wc.lpszClassName = RDOC_WINDOW_CLASS_NAME;
  BOOL ret = RegisterClass(&wc);
  if(!ret && GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
  {
    return TRUE;
  }

  return ret;
}

//-----------------------------------------------------------------------------
// CreateWnd
//-----------------------------------------------------------------------------
HWND CreateWnd(HINSTANCE hInstance, HINSTANCE hPrevInstance, uint32_t PosX, uint32_t PosY,
               uint32_t Width, uint32_t Height, DWORD Style, DWORD ExtendedStyle)
{
  // Need to adjust first, to see if it fits the screen.
  RECT WindowRect = {(LONG)PosX, (LONG)PosY, (LONG)(PosX + Width), (LONG)(PosY + Height)};
  AdjustWindowRectEx(&WindowRect, Style, NULL, ExtendedStyle);
  Width = WindowRect.right - WindowRect.left;
  Height = WindowRect.bottom - WindowRect.top;

  return CreateWindowEx(ExtendedStyle, RDOC_WINDOW_CLASS_NAME, RDOC_WINDOW_TITLE, Style, PosX, PosY,
                        Width, Height, (HWND)NULL, (HMENU)NULL, hInstance, (LPVOID)NULL);
}

void CreateResources()
{
  RegisterWndClass(appInstance, CS_HREDRAW | CS_VREDRAW);
  // Resolution Width and Height are declared in gen_variables
  appHwnd = CreateWnd(appInstance, NULL, 0, 0, resolutionWidth, resolutionHeight,
                      WS_BORDER | WS_DLGFRAME | WS_GROUP | WS_OVERLAPPED | WS_POPUP | WS_SIZEBOX |
                          WS_SYSMENU | WS_TILED | WS_VISIBLE,
                      0);

  SetWindowTextA(appHwnd, "RenderDoc Frame Loop: Creating Resources");
  main_create();
  SetWindowTextA(appHwnd, "RenderDoc Frame Loop: Initializing Resources");
  main_init();
}

//-----------------------------------------------------------------------------
// ReleaseResources
//-----------------------------------------------------------------------------
void ReleaseResources()
{
  main_release();
}

//-----------------------------------------------------------------------------
// GetTimestampMillis
//-----------------------------------------------------------------------------
double GetTimestampMilliseconds()
{
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  return 1e3 * ((double)counter.QuadPart) / performanceCounterFrequency.QuadPart;
}

//-----------------------------------------------------------------------------
// Render
//-----------------------------------------------------------------------------
void Render()
{
  double ts_pre_reset = GetTimestampMilliseconds();
  main_prereset();
  double ts_start = GetTimestampMilliseconds();
  main_render();
  main_postreset();
  double ts_end = GetTimestampMilliseconds();
  double frame_time = ts_end - ts_start;
  double frame_time_with_reset = ts_end - ts_pre_reset;

  frames++;

  accumTimeWithReset += frame_time_with_reset;
  accumTime += frame_time;
  avgTimeWithReset = accumTimeWithReset / frames;
  avgTime = accumTime / frames;
  avgFPSWithReset = 1000.0 / avgTimeWithReset;
  avgFPS = 1000.0 / avgTime;

  if(frames % 1 == 0)
  {
    char str[256];
    sprintf(str, "%s Avg Time [%f / %f] Avg FPS [%f /%f]", "RenderDoc Frame Loop", avgTimeWithReset,
            avgTime, avgFPSWithReset, avgFPS);
    SetWindowTextA(appHwnd, str);
  }
}

//-----------------------------------------------------------------------------
// ProcessMessages
//-----------------------------------------------------------------------------
static void ProcessMessages(bool &quit)
{
  MSG msg;
  if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    if(msg.message == WM_QUIT)
    {
      quit = true;
    }
    else
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}

//-----------------------------------------------------------------------------
// ParseCommandLine
//-----------------------------------------------------------------------------
static bool ParseCommandLine()
{
  for(int i = 1; i < __argc; ++i)
  {
    if(0 == strcmp(__argv[i], "-repeat"))
    {
      ++i;
      if(i >= __argc)
      {
        return false;
      }
      frameLoops = atoi(__argv[i]);
    }
    else if(0 == strcmp(__argv[i], "-reset"))
    {
      resourceReset = true;
    }
    else
    {
      // Unknown command
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------
// Usage
//-----------------------------------------------------------------------------
static void Usage()
{
  const wchar_t *usage =
      L"Options:\n"
      L"-repeat N    -- Number of frames to run\n"
      L"-reset       -- Perform a state reset in between frames\n";

  MessageBox(NULL, usage, L"Invalid command line", MB_ICONEXCLAMATION);
}

//-----------------------------------------------------------------------------
// WinMain
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  bool quit = false;

  appInstance = hInstance;

  // Parse the command line arguments
  if(!ParseCommandLine())
  {
    Usage();
    return EXIT_FAILURE;
  }

  try
  {
    CreateResources();

    QueryPerformanceFrequency(&performanceCounterFrequency);

    int repeatIteration = 0;
    while(frameLoops == -1 || repeatIteration < frameLoops)
    {
      ProcessMessages(quit);
      if(quit)
      {
        break;
      }

      Render();

      repeatIteration = (std::max)(0, repeatIteration + 1);
    }
  }
  catch(std::exception &e)
  {
    if(automated)
    {
      fprintf(stderr, "Error: %s", e.what());
    }
    else
    {
      std::string errorMessage(e.what());
      std::wstring errorMessageW(errorMessage.begin(), errorMessage.end());
      MessageBox(NULL, errorMessageW.c_str(), L"Error", MB_ICONEXCLAMATION);
    }
  }

  ReleaseResources();
  return EXIT_SUCCESS;
}

#endif    // if _WIN32
