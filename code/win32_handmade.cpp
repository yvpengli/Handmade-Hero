#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static


// TODO: this is a global for now
global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal void RenderWeirdGradient(int XOffset, int YOffset)
{
  int Width = BitmapWidth;
  int Height = BitmapHeight;

  int Pitch = Width * BytesPerPixel;
  uint8_t *Row = (uint8_t *)BitmapMemory;
  for (int y = 0; y < BitmapHeight; y ++)
  {
    uint32_t *Pixel = (uint32_t *) Row;
    for (int x = 0; x < BitmapWidth; x ++ ) 
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
    Row += Pitch;
  }
}

internal void Win32ResizeDIBSection(int Width, int Height)
{
  if(BitmapMemory)
  {
    VirtualFree(BitmapMemory, 0, MEM_RELEASE);
  }

  BitmapWidth = Width;
  BitmapHeight = Height;

  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = BitmapWidth;
  BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;

  // size of one pixel is 32bit/4Byte.
  int BytesPerPixel = 4;
  int BitmapMemorySize = (BitmapWidth*BitmapHeight)*BytesPerPixel;
  BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

}

internal void Win32UpdateWindow(HDC DeviceContext, RECT *WindowRect, int X, int Y, int Width, int Height)
{
  int WindowWidth = WindowRect->right - WindowRect->left;
  int WindowHeight = WindowRect->bottom - WindowRect->top;

  int result = StretchDIBits(DeviceContext,
    0,0,BitmapWidth,BitmapHeight,
    0,0,WindowWidth,WindowHeight,
    BitmapMemory,
    &BitmapInfo,
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
      RECT ClientRect;
      GetClientRect(hwnd, &ClientRect);
      int Width = ClientRect.right - ClientRect.left;
      int Height = ClientRect.bottom - ClientRect.top;
      Win32ResizeDIBSection(Width, Height);
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

      RECT ClientRect;
      GetClientRect(hwnd, &ClientRect);
      Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
      
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
  WNDCLASS WindowClass = {};

  WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = WindowProcCallBack;
  WindowClass.hInstance = hInstance;
  // WindowClass.hIcon;
  WindowClass.lpszClassName = "myHandmadeHeroWindowClass";

  if (RegisterClass(&WindowClass)) {
    HWND WindowHandle = 
      CreateWindowEx(
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

        RenderWeirdGradient(XOffset, YOffset);
        HDC DeviceContext = GetDC(WindowHandle);
        RECT ClientRect;
        GetClientRect(WindowHandle, &ClientRect);
        int WindowWidth = ClientRect.right - ClientRect.left;
        int WindowHeight = ClientRect.bottom - ClientRect.top;
        Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
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

