#include <io.h>
#include <fcntl.h>

#include <windows.h>

#include "png.h"

#include "resource.h"

#define RET_ERROR 1
#define PNG_HEADER_SIZE	8
#define BUF_TEMP_FILENAME 512

#define alpha_composite(composite, fg, alpha, bg) {					\
    USHORT temp = ((USHORT)(fg)*(USHORT)(alpha) +                   \
    (USHORT)(bg)*(USHORT)(255 - (USHORT)(alpha)) + (USHORT)128);	\
    (composite) = (UCHAR)((temp + (temp >> 8)) >> 8);               \
}

static ULONG ulWinImageRowbytes;
static UCHAR* ucpImageData;
static UCHAR* ucpDecodeImageData;
static BITMAPINFO biMinfo;

static ULONG ulPngWidth, ulPngHeight, ulImageRowBytes;
static INT nBitDepth, nColorType, nImageChannels;
png_color_16p pBackground;
static UCHAR nBgRed, nBgGreen, nBgBlue, nBgAlpha;
UCHAR*	ucpReadImageData;

static HWND hMainWnd;
LPCTSTR szAppName = TEXT("ShowPNGWindow");
HBITMAP		hbitmap;
HDC			hInitDc, hmemdc;


static int rpng_win_display_image()
{
	UCHAR *src, *dest;
    UCHAR r, g, b, a;
    ULONG i, row, lastrow;
    RECT rect;

    for (lastrow = row = 0;  row < ulPngHeight;  ++row) 
	{
        src = ucpDecodeImageData + row * ulImageRowBytes;
        dest = ucpImageData + row * ulWinImageRowbytes;

        if (nImageChannels == 3)
		{
            for (i = ulPngWidth;  i > 0;  --i) 
			{
                r = *src++;
                g = *src++;
                b = *src++;
				a = 255;	// If no alpha channel, set no transparent
                *dest++ = b;
                *dest++ = g;
                *dest++ = r;
				*dest++ = a;
            }
        } 
		else
		{
            for (i = ulPngWidth;  i > 0;  --i) 
			{
                r = *src++;
                g = *src++;
                b = *src++;
                a = *src++;

                if (a == 255) 
				{
                    *dest++ = b;
                    *dest++ = g;
                    *dest++ = r;
                } 
				else if (a == 0) 
				{
                    *dest++ = nBgBlue;
                    *dest++ = nBgGreen;
                    *dest++ = nBgRed;
                } 
				else 
				{
                    alpha_composite(*dest++, b, a, nBgBlue);
                    alpha_composite(*dest++, g, a, nBgGreen);
                    alpha_composite(*dest++, r, a, nBgRed);
                }

				*dest++ = a;	// Alpha is on the highest byte
            }
        }
        
		// Display step by 0xf Lines
        if (((row+1) & 0xf) == 0) 
		{
            rect.left = 0L;
            rect.top = (LONG)lastrow;
            rect.right = (LONG)ulPngWidth;
            rect.bottom = (LONG)lastrow + 16L;
            InvalidateRect(hMainWnd, &rect, FALSE);
            UpdateWindow(hMainWnd);
            lastrow = row + 1;
        }
    }

    if (lastrow < ulPngHeight) 
	{
        rect.left = 0L;
        rect.top = (LONG)lastrow;
        rect.right = (LONG)ulPngWidth;
        rect.bottom = (LONG)ulPngHeight;
		InvalidateRect(hMainWnd, &rect, FALSE);
        UpdateWindow(hMainWnd);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC         hdc;
    PAINTSTRUCT ps;
	BLENDFUNCTION bf;

    switch (msg) 
	{
        case WM_CREATE:
            return 0;

		case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);

			SelectObject(hmemdc, hbitmap);

			//BitBlt(hdc, 0, 0, ulPngWidth, ulPngHeight, hmemdc, 0, 0, SRCCOPY);
								
			
			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.AlphaFormat = AC_SRC_ALPHA;
			bf.SourceConstantAlpha = 255;
			if(!AlphaBlend(hdc, 0, 0, ulPngWidth, ulPngHeight, hmemdc, 0, 0, ulPngWidth, ulPngHeight, bf))
			{
				MessageBox(NULL, TEXT("Error"), TEXT("Af failed"), MB_OK);
			}
						
			EndPaint(hwnd, &ps);
            return 0;

        case WM_CHAR:
            switch (wParam) 
			{
                case 'q':
                case 'Q':
                case 0x1B:
                    PostQuitMessage(0);
            }
            return 0;

        case WM_LBUTTONDOWN:
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	
	static PBYTE		pbytePngByte;
	HGLOBAL				hPNGDATA;

	BYTE				byteHeader[8];
	DWORD				dwPngFileSize;

	DWORD				dwRetVal;
	UINT				uRetVal;
	BOOL				bRetVal;
	INT					nRetVal;
	TCHAR				lpszPathBuffer[BUF_TEMP_FILENAME];
	TCHAR				lpszTempName[BUF_TEMP_FILENAME];
	HANDLE				hTempFile;
	FILE*				pfPngFile;

	UCHAR*				ucpDest;
	UINT				i, j;
	INT					nExtraWidth, nExtraHeight;

	WNDCLASSEX			wndclass;
	MSG					msg;

	png_structp			pPngStruct;
	png_infop			pPngInfo;
	DOUBLE				LUT_exponent, CRT_exponent, DISP_exponent, dbGAMA;
	/*CHAR*				cpSreenGAMA;
	size_t				stLen;*/
	png_uint_32			pngRowBytes;
	png_bytepp			pngRowPointers = NULL;

	hPNGDATA = LoadResource(hInstance, FindResource(hInstance, MAKEINTRESOURCE(IDB_PNGSPASH), TEXT("PNG")));
	dwPngFileSize = SizeofResource(hInstance, FindResource(hInstance, MAKEINTRESOURCE(IDB_PNGSPASH), TEXT("PNG")));
	pbytePngByte = (PBYTE)LockResource(hPNGDATA);

	//Check if the resource is a png file
	CopyMemory(byteHeader, pbytePngByte, PNG_HEADER_SIZE);

	if (png_sig_cmp(byteHeader, 0, PNG_HEADER_SIZE))
	{
		return RET_ERROR;
	}

	//Create a temp file of the png file
	dwRetVal = GetTempPath(BUF_TEMP_FILENAME, lpszPathBuffer);

	if (dwRetVal > BUF_TEMP_FILENAME || (dwRetVal == 0))
    {
        return RET_ERROR;
    }

	uRetVal = GetTempFileName(lpszPathBuffer, TEXT("PNG"), 0, lpszTempName);

    if (uRetVal == 0)
    {
		return RET_ERROR;
    }

	hTempFile = CreateFile(lpszTempName,
						   GENERIC_READ | GENERIC_WRITE,
						   0,
						   NULL,
						   CREATE_ALWAYS,
						   FILE_ATTRIBUTE_NORMAL,
						   NULL);

	if (hTempFile == INVALID_HANDLE_VALUE) 
    { 
		return RET_ERROR;
    }

	bRetVal = WriteFile(hTempFile, pbytePngByte, dwPngFileSize, &dwRetVal, 0);

	if(!bRetVal || dwRetVal != dwPngFileSize)
	{
		return RET_ERROR;
	}

	//Convert the File Handle of windows to FILE pointer of C
	nRetVal = _open_osfhandle((intptr_t)hTempFile, _O_RDONLY);

	if(nRetVal == -1)
	{
		return RET_ERROR;
	}

	pfPngFile = _fdopen(nRetVal, "rb");

	if(!pfPngFile)
	{
		return RET_ERROR;
	}

	rewind(pfPngFile);

	//Proceed with the png FILE pointer
	pPngStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	if (!pPngStruct)
	{
		return RET_ERROR;
	}

	pPngInfo = png_create_info_struct(pPngStruct);

	if (!pPngInfo)
	{
		return RET_ERROR;
	}

	//Read the png Solution
	if (setjmp(png_jmpbuf(pPngStruct)))
	{
		return RET_ERROR;
	}

	png_init_io(pPngStruct, pfPngFile);
	png_read_info(pPngStruct, pPngInfo);

	png_get_IHDR(pPngStruct, pPngInfo, &ulPngWidth, &ulPngHeight, &nBitDepth, &nColorType, NULL, NULL, NULL);

	//Read the png background
	if (setjmp(png_jmpbuf(pPngStruct)))
	{
		return RET_ERROR;
    }

	//If the background is tansparent
    if (!png_get_valid(pPngStruct, pPngInfo, PNG_INFO_bKGD))
	{    
		nBgRed = nBgGreen = nBgBlue = 0;
		nBgAlpha = 0;	//The bitmap is totallly transparent
	}
	else
	{
		png_get_bKGD(pPngStruct, pPngInfo, &pBackground);
		    
		if (nBitDepth == 16)
		{
			nBgRed = pBackground->red >> 8;
			nBgGreen = pBackground->green >> 8;
			nBgBlue = pBackground->blue >> 8;
		} 
		else if (nColorType == PNG_COLOR_TYPE_GRAY && nBitDepth < 8) 
		{
		   if (nBitDepth == 1)
			    nBgRed = nBgGreen = nBgBlue = pBackground->gray? 255 : 0;
		   else if (nBitDepth == 2)
				nBgRed = nBgGreen = nBgBlue = (255/3) * pBackground->gray;
		   else
				nBgRed = nBgGreen = nBgBlue = (255/15) * pBackground->gray;
		} 
		else 
		{
			nBgRed = (UCHAR)pBackground->red;
		    nBgGreen = (UCHAR)pBackground->green;
			nBgBlue = (UCHAR)pBackground->blue;
		}

		nBgAlpha = 255;	//The bitmap is totally opaque
	}

	//Create the BITMAPINFOHEADER
	ulWinImageRowbytes = ((4*ulPngWidth) >> 2) << 2;	//For a alpha byte

	memset(&biMinfo, 0, sizeof(BITMAPINFO));

    biMinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    biMinfo.bmiHeader.biWidth = ulPngWidth;
    biMinfo.bmiHeader.biHeight = -((long)ulPngHeight);
    biMinfo.bmiHeader.biPlanes = 1;
    biMinfo.bmiHeader.biBitCount = 32;	//More 8 Bit for Alpha
    biMinfo.bmiHeader.biCompression = BI_RGB;
	biMinfo.bmiHeader.biSizeImage = ulWinImageRowbytes * ulPngHeight;


	//Fill the Window Class Parameters
	memset(&wndclass, 0, sizeof(wndclass));

    wndclass.cbSize = sizeof(wndclass);
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);	//Set Background
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&wndclass);

	//Create the Window
	nExtraWidth = 2*(GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CXDLGFRAME));
    nExtraHeight = 2*(GetSystemMetrics(SM_CYBORDER) + GetSystemMetrics(SM_CYDLGFRAME)) + GetSystemMetrics(SM_CYCAPTION);

    hMainWnd = CreateWindow(szAppName, 
							szAppName, 
							WS_OVERLAPPEDWINDOW, 
							CW_USEDEFAULT, 
							CW_USEDEFAULT, 
							ulPngWidth + nExtraWidth, 
							ulPngHeight + nExtraHeight, 
							NULL, 
							NULL, 
							hInstance, 
							NULL);

	//Create a bitmap Section
	hInitDc = GetDC(hMainWnd);
	hmemdc = CreateCompatibleDC(hInitDc);
	hbitmap = CreateDIBSection(hmemdc, &biMinfo, DIB_RGB_COLORS, (void **)&ucpImageData, NULL, 0x0);
	ReleaseDC(hMainWnd, hInitDc);

	memset(ucpImageData, 0, ulWinImageRowbytes * ulPngHeight);

	//Fill the background color of bitmap
	for (j = 0;  j < ulPngHeight;  ++j) 
	{
        ucpDest = ucpImageData + j*ulWinImageRowbytes;
        for (i = ulPngWidth;  i > 0;  --i) 
		{
			*ucpDest++ = nBgBlue;
            *ucpDest++ = nBgGreen;
            *ucpDest++ = nBgRed;
			*ucpDest++ = nBgAlpha;
        }
    }

    ShowWindow(hMainWnd, SW_SHOW);
    UpdateWindow(hMainWnd);

	//Decode the png data
	CRT_exponent = 2.2;
	LUT_exponent = 1.0;

	/*if((_dupenv_s(&cpSreenGAMA, &stLen, "SCREEN_GAMMA")) == 0)
	{
		DISP_exponent = atof(*cpSreenGAMA);
	}
	else
	{
		DISP_exponent = CRT_exponent * LUT_exponent;
	}*/

	DISP_exponent = CRT_exponent * LUT_exponent;

	if (setjmp(png_jmpbuf(pPngStruct)))
	{
		return RET_ERROR;
    }


    if (nColorType == PNG_COLOR_TYPE_PALETTE)
	{
        png_set_expand(pPngStruct);
	}
	
	if (nColorType == PNG_COLOR_TYPE_GRAY && nBitDepth < 8)
	{
        png_set_expand(pPngStruct);
	}

    if (png_get_valid(pPngStruct, pPngInfo, PNG_INFO_tRNS))
	{
        png_set_expand(pPngStruct);
	}

    if (nBitDepth == 16)
	{
        png_set_strip_16(pPngStruct);
	}

    if (nColorType == PNG_COLOR_TYPE_GRAY || nColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
        png_set_gray_to_rgb(pPngStruct);
	}


	if (png_get_gAMA(pPngStruct, pPngInfo, &dbGAMA))
	{
        png_set_gamma(pPngStruct, DISP_exponent, dbGAMA);
	}

	png_read_update_info(pPngStruct, pPngInfo);

    ulImageRowBytes = pngRowBytes = png_get_rowbytes(pPngStruct, pPngInfo);
    nImageChannels = (int)png_get_channels(pPngStruct, pPngInfo);

    if ((ucpReadImageData = (UCHAR*)malloc(pngRowBytes * ulPngHeight)) == NULL) 
	{
		return RET_ERROR;
    }

    if ((pngRowPointers = (png_bytepp)malloc(ulPngHeight * sizeof(png_bytep))) == NULL) 
	{
		return RET_ERROR;
    }

	for (i = 0;  i < ulPngHeight;  ++i)
	{
        pngRowPointers[i] = ucpReadImageData + i*ulImageRowBytes;
	}

    png_read_image(pPngStruct, pngRowPointers);

	free(pngRowPointers);
    pngRowPointers = NULL;

    png_read_end(pPngStruct, NULL);

	ucpDecodeImageData = ucpReadImageData;

	_close(nRetVal);

	if(!DeleteFile(lpszTempName))
	{
		MessageBox(NULL, TEXT("ERROR"), TEXT("ERROR"), MB_OK);
	}

	rpng_win_display_image();

	while (GetMessage(&msg, NULL, 0, 0)) 
	{
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (ucpDecodeImageData) {
        free(ucpDecodeImageData);
        ucpDecodeImageData = NULL;
    }

	//DeleteObject(hbitmap);
	//DeleteDC(hmemdc);

    return msg.wParam;

	return 0;
}