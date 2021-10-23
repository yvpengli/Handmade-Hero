#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

struct win32_offscreen_buffer
{
  // a struct to store Bitmap informations
  BITMAPINFO Info;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel = 4;
};

struct win32_window_dimension
{
  int Width;
  int Height;
};

win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
  win32_window_dimension Result;

  RECT ClientRect;
  GetClientRect(Window, &ClientRect);
  Result.Width = ClientRect.right - ClientRect.left;
  Result.Height = ClientRect.bottom - ClientRect.top;

  return(Result);
}

// TODO: this is a global for now
global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

internal void RenderWeirdGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset)
{
  // don't know if pass Buffer as value is better or as point is better
  int Width = Buffer.Width;
  int Height = Buffer.Height;

  uint8_t *Row = (uint8_t *)Buffer.Memory;
  for (int y = 0; y < Height; y ++)
  {
    uint32_t *Pixel = (uint32_t *) Row;
    for (int x = 0; x < Width; x ++ ) 
    {
      /*
        Pixel in memory: BBGGRRxx
        LITTLE ENDIAN ARCHITECTURE
        Actually in Register: xxRRGGBB
      */
      uint8_t Blue = (x + XOffset);
      uint8_t Green = (y + YOffset);

      *Pixel++ = ((Green<<8) | Blue);
    }
    Row += Buffer.Pitch;
  }
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
  if(Buffer->Memory)
  {
    VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
  }

  Buffer->Width = Width;
  Buffer->Height = Height;

  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biWidth = Buffer->Width;
  Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
  Buffer->Info.bmiHeader.biPlanes = 1;
  Buffer->Info.bmiHeader.biBitCount = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;

  // size of one pixel is 32bit/4Byte.
  Buffer->BytesPerPixel = 4;
  int BitmapMemorySize = (Buffer->Width * Buffer->Height)*Buffer->BytesPerPixel;
  Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;   

}

internal void 
Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, 
                  win32_offscreen_buffer Buffer,
                  int X, int Y, int Width, int Height)
{
  // TODO: Aspect ratio correction
  StretchDIBits(DeviceContext,
    0,0,WindowWidth,WindowHeight,
    0,0,Buffer.Width, Buffer.Height,
    Buffer.Memory,
    &Buffer.Info,
    DIB_RGB_COLORS,SRCCOPY);
  
}

LRESULT CALLBACK WindowProcCallBack(
  HWND hwnd, 
  UINT uMsg, 
  WPARAM wParam, 
  LPARAM lParam 
) {
  LRESULT Result;
  switch(uMsg) {
    case WM_SIZE:
    { 
      OutputDebugStringA("WM_SIZE\n");
    } break;
    case WM_DESTROY:
    {
      Running = false;
      OutputDebugStringA("WM_DESTROY\n");
    } break;
    case WM_CLOSE:
    {
      Running = false;
      OutputDebugStringA("WM_CLOSE\n");
    } break;
    case WM_ACTIVATEAPP:
    {
      // Sent when a window belonging to a different application than the active window is about to be activated.
      OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;
    case WM_PAINT: 
    {
      PAINTSTRUCT Paint;
      HDC DeviceContext = BeginPaint(hwnd, &Paint);
      int X = Paint.rcPaint.left;
      int Y = Paint.rcPaint.top;
      int Width = Paint.rcPaint.right - Paint.rcPaint.left;
      int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

      win32_window_dimension Dimension = Win32GetWindowDimension(hwnd);
      Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
                                GlobalBackBuffer,
                                X, Y, Width, Height);
      
      EndPaint(hwnd, &Paint);
      OutputDebugStringA("WM_PAINT\n");
    } break;
    default:
    {
      Result = DefWindowProcA(hwnd, uMsg, wParam, lParam);
    } break;
  }

  return (Result);

} 



int CALLBACK WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nShowCmd
){
  WNDCLASSA WindowClass = {};

  Win32ResizeDIBSection(&GlobalBackBuffer, 1200, 720);

  WindowClass.style =CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = WindowProcCallBack;
  WindowClass.hInstance = hInstance;
  // WindowClass.hIcon;
  WindowClass.lpszClassName = "myHandmadeHeroWindowClass";

  if (RegisterClassA(&WindowClass)) {
    HWND WindowHandle = 
      CreateWindowExA(
        0,
        WindowClass.lpszClassName,
        "Handmade Hero",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        hInstance,
        0);
    if (WindowHandle) {
      Running = true;
      int XOffset = 0;
      int YOffset = 0;

      while (Running) {
        MSG Message;
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
          if (Message.message == WM_QUIT) 
          {
            Running = false;
          }
          TranslateMessage(&Message);
          DispatchMessage(&Message);
        }

        RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);
        HDC DeviceContext = GetDC(WindowHandle);
        win32_window_dimension Dimension = Win32GetWindowDimension(WindowHandle);
        Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackBuffer, 0, 0, Dimension.Width, Dimension.Height);
        ReleaseDC(WindowHandle, DeviceContext);

        XOffset ++;
        YOffset ++;
      }

    } else {
      // TODO
    }
  } else {
      // TODO
  }
  
  return (0);
}

