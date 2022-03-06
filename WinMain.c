
#include <windows.h>
#include <stdint.h>

// Globals.
static int running;

static void *bitmapMemory;
static BITMAPINFO bitmapInfo;
static int bytePerPixel = 4;
static int bitmapWidth;
static int bitmapHeight;


typedef struct windowDimension{
	int width;
	int height;
}winDimension;

enum styles {_Tiles, _Gradient};

// Get the window dimensions
winDimension GetWinDimension(HWND window){/*{{{*/
	winDimension winDimension;

	RECT rect;
	GetClientRect(window, &rect);	

	winDimension.width = rect.right - rect.left;
	winDimension.height = rect.bottom - rect.top;
	return(winDimension);
}/*}}}*/

// Resize the bitmap buffer
void InitDIBSection(int width, int height){/*{{{*/

	if(bitmapMemory){
		VirtualFree(bitmapMemory, 0, MEM_RELEASE);
	}

	bitmapWidth = width;
	bitmapHeight = height;

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = bitmapWidth;
	bitmapInfo.bmiHeader.biHeight = -bitmapHeight;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	int bitmapSize = (bitmapWidth * bitmapHeight) * bytePerPixel;
	bitmapMemory = VirtualAlloc(0, bitmapSize, MEM_COMMIT, PAGE_READWRITE);
}/*}}}*/

void Render(int xOffset, int yOffset, int style){/*{{{*/

	int pitch = bitmapWidth * bytePerPixel;
	uint8_t *row = (uint8_t *)bitmapMemory;

	switch(style){
		case _Tiles:
		{
			for(int y = 0; y < bitmapHeight; y++){
				uint32_t *pixel = (uint32_t *)row;

				for(int x = 0; x < bitmapWidth; x++){
					uint8_t blue = x + xOffset;
					uint8_t red = y + yOffset;
					uint8_t green = 150;

					//*pixel++ = 0xffb703;
					*pixel++ = ((blue << 8) | (red << 8) | (green << 4));
				}
				row += pitch;
			}
		}break;

		case _Gradient:
		{
			for(int y = 0; y < bitmapHeight; y++){
				uint8_t *pixel = (uint8_t *)row;

				for(int x = 0; x < bitmapWidth; x++){
					
					// Blue
					*pixel++ = 75;

					// Green
					*pixel++ = (y * 0.25);
					
					// Red
					*pixel++ = (x * 0.22);

					pixel++;
				}
				row += pitch;
			}
		}break;

	}
}/*}}}*/

// Draw bitmap buffer into the screen
void Win32DrawBuffer(HDC deviceContext, int bitmapWidth, int bitmapHeight, int windowWidth, int windowHeight){/*{{{*/

  StretchDIBits(deviceContext,
  0, 0, windowWidth, windowHeight, 
  0, 0, bitmapWidth, bitmapHeight,
  bitmapMemory,
  &bitmapInfo,
  DIB_RGB_COLORS, SRCCOPY);

}/*}}}*/

// WinProc
LRESULT CALLBACK Winproc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam){/*{{{*/

	LRESULT result = 0;
	switch(msg){

		case WM_SIZE:
		{
			winDimension winDim = GetWinDimension(window);
			InitDIBSection(winDim.width, winDim.height);
		}break;

		case WM_CLOSE:
		{
			running = 0;
		}break;

		case WM_DESTROY:
		{
			running = 0;
		}break;

		case WM_PAINT:
		{
			// Start painting
			PAINTSTRUCT paint;

			HDC deviceContext = BeginPaint(window, &paint);

			Win32DrawBuffer(deviceContext,
			bitmapWidth, bitmapHeight,
			paint.rcPaint.right, paint.rcPaint.bottom);

			EndPaint(window, &paint);
		}break;

		default:
		{
			result = DefWindowProc(window, msg, wParam, lParam);	
		}
	}

	return result; 
}/*}}}*/

// Main API Entry
int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow){/*{{{*/

	// Create a class
	WNDCLASS class = {0};
	class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	class.lpfnWndProc = Winproc;
	class.hInstance = instance;
	class.lpszClassName = "WindowClass";
	class.hCursor = LoadCursor(NULL, IDC_ARROW);

	// Register the class
	if(RegisterClass(&class)){

		// Create a window
		HWND window = CreateWindow(class.lpszClassName,
		"Rendering demo", 
		WS_VISIBLE|
		WS_CAPTION|
		WS_SYSMENU,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0, 
		instance, 
		0);

		if(window){
			running = 1;
			int xOffset = 0;
			int yOffset = 0;

			while(running){

				MSG msg;
				while(PeekMessage(&msg, window, 0, 0, PM_REMOVE)){
					if(msg.message == WM_QUIT){
						running = 0;
					}
					TranslateMessage(&msg);
					DispatchMessage(&msg);		// Send the message to window proc
				}

				// Init the buffer
				Render(xOffset, yOffset, _Tiles);

				HDC deviceContext = GetDC(window);
				winDimension winDim = GetWinDimension(window);

				// Draw the buffer
				Win32DrawBuffer(deviceContext, bitmapWidth, bitmapHeight, winDim.width, winDim.height);
				xOffset++;
				yOffset++;

				ReleaseDC(window, deviceContext);
			}

		}
	}

	return 0;
}/*}}}*/


