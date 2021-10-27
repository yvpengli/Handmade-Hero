#include <windows.h>
#include <stdint.h>
#include <Xinput.h>

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

// XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
  return(0);
}

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
  return(0);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

internal void 
Win32LoadXInput(void)
{
  HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");
  if (XInputLibrary)
  {
    XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
    XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
  }
}

internal win32_window_dimension
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
                  win32_offscreen_buffer *Buffer)
{
  // TODO: Aspect ratio correction
  StretchDIBits(DeviceContext,
    0,0,WindowWidth,WindowHeight,
    0,0,Buffer->Width, Buffer->Height,
    Buffer->Memory,
    &Buffer->Info,
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
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
      uint32_t VKCode = wParam;
      bool WasDown = ((lParam & ( 1 << 30 )) != 0);
      bool IsDown = ((lParam & ( 1 << 31 )) == 0);
      if ( WasDown != IsDown )
      {
        if (VKCode == 'W')
        {
          OutputDebugStringA("W\n");
        } 
        else if (VKCode == 'A')
        {
          OutputDebugStringA("A\n");
        }
        else if (VKCode == 'S')
        {
          OutputDebugStringA("S\n");
        } 
        else if (VKCode == 'D')
        {
          OutputDebugStringA("D\n");
        } 
        else if (VKCode == 'Q')
        {
          OutputDebugStringA("Q\n");
        }      
        else if (VKCode == 'E')
        {
          OutputDebugStringA("E\n");
        }
        else if (VKCode == VK_UP)
        {
        }
        else if (VKCode == VK_LEFT)
        {
        }
        else if (VKCode == VK_DOWN)
        {
        }
        else if (VKCode == VK_RIGHT)
        {
        }
        else if (VKCode == VK_ESCAPE)
        {
          OutputDebugStringA("ESCAPE: ");
          if (IsDown) 
          {
            OutputDebugStringA("IsDown ");
          }
          if (WasDown)
          {
            OutputDebugStringA("WasDown ");
          }
          OutputDebugStringA("\n");
        }
        else if (VKCode == VK_SPACE)
        {
        }        
      }

    } break;


    case WM_PAINT: 
    {
      PAINTSTRUCT Paint;
      HDC DeviceContext = BeginPaint(hwnd, &Paint);
      win32_window_dimension Dimension = Win32GetWindowDimension(hwnd);
      Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
                                &GlobalBackBuffer);
      
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
  // Load XInput Library
  Win32LoadXInput();

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
      // Since we specified CS_OWND, we can just
      // get one device context and use it forever because we are 
      // not sharing it with anyone
      HDC DeviceContext = GetDC(WindowHandle);

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
        // TODO: Should we poll this more frequently
        for (DWORD ControllerIndex = 0;
            ControllerIndex < XUSER_MAX_COUNT;
            ControllerIndex ++)
        {
          XINPUT_STATE ControllerState;
          if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
          {
            // NOTE: This controller is Plugged in
            XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

            bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
            bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
            bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
            bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
            bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
            bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

            int16_t StickX = Pad->sThumbRX;
            int16_t StickY = Pad->sThumbRY;

            if (AButton)
            {
              YOffset += 2;
            }
          }
          else 
          {
            // NOTE: This controller is not available
          }
        }

        RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);
        win32_window_dimension Dimension = Win32GetWindowDimension(WindowHandle);
        Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackBuffer);

        XOffset ++;
        
      }

    } else {
      // TODO
    }
  } else {
      // TODO
  }
  
  return (0);
}

