//------------------------------------------------------------------------------
// <copyright file="DrawDevice.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// Manages the drawing of bitmap data

#include "stdafx.h"
#include "DrawDevice.h"

int padding = 0;

inline LONG Width( const RECT& r )
{
    return r.right - r.left;
}

inline LONG Height( const RECT& r )
{
    return r.bottom - r.top;
}

//-------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------
DrawDevice::DrawDevice() : 
    m_hwnd(0),
    m_sourceWidth(0),
    m_sourceHeight(0),
    m_stride(0),
    m_pD2DFactory(NULL), 
    m_pRenderTarget(NULL),
    m_pBitmap(0)
{
}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------
DrawDevice::~DrawDevice()
{
    DiscardResources();
    SafeRelease(m_pD2DFactory);
}

//-------------------------------------------------------------------
// EnsureResources
//
// Ensure necessary Direct2d resources are created
//-------------------------------------------------------------------
HRESULT DrawDevice::EnsureResources()
{
    HRESULT hr = S_OK;

    if ( !m_pRenderTarget )
    {
        D2D1_SIZE_U size = D2D1::SizeU( m_sourceWidth, m_sourceHeight );

        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
        rtProps.pixelFormat = D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
        rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

        // Create a Hwnd render target, in order to render to the window set in initialize
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            rtProps,
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRenderTarget
            );

        if ( FAILED( hr ) )
        {
            return hr;
        }

        // Create a bitmap that we can copy image data into and then render to the target
        hr = m_pRenderTarget->CreateBitmap(
            D2D1::SizeU( m_sourceWidth, m_sourceHeight ), 
            D2D1::BitmapProperties( D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE) ),
            &m_pBitmap 
            );

        if ( FAILED( hr ) )
        {
            SafeRelease( m_pRenderTarget );
            return hr;
        }
    }

    return hr;
}

//-------------------------------------------------------------------
// DiscardResources
//
// Dispose Direct2d resources 
//-------------------------------------------------------------------
void DrawDevice::DiscardResources( )
{
    SafeRelease(m_pRenderTarget);
    SafeRelease(m_pBitmap);
}

//-------------------------------------------------------------------
// Initialize
//
// Set the window to draw to, video format, etc.
//-------------------------------------------------------------------
bool DrawDevice::Initialize( HWND hwnd, ID2D1Factory * pD2DFactory, int sourceWidth, int sourceHeight, int Stride )
{
    m_hwnd = hwnd;

    // One factory for the entire application so save a pointer here
    m_pD2DFactory = pD2DFactory;

    m_pD2DFactory->AddRef( );

    // Get the frame size
    m_stride = Stride;

    m_sourceWidth = sourceWidth;
    m_sourceHeight = sourceHeight;
    
    return true;
}

//-------------------------------------------------------------------
// DrawFrame
//
// Draw the video frame.
//-------------------------------------------------------------------
bool DrawDevice::Draw( BYTE * pBits, unsigned long cbBits )
{
    // incorrectly sized image data passed in
    if ( cbBits < ((m_sourceHeight - 1) * m_stride) + (m_sourceWidth * 4) )
    {
        return false;
    }

    // create the resources for this draw device
    // they will be recreated if previously lost
    HRESULT hr = EnsureResources( );

    if ( FAILED( hr ) )
    {
        return false;
    }
    
    // Copy the image that was passed in into the direct2d bitmap
    hr = m_pBitmap->CopyFromMemory( NULL, pBits, m_stride );

    if ( FAILED( hr ) )
    {
        return false;
    }
       
    m_pRenderTarget->BeginDraw();

    // Draw the bitmap stretched to the size of the window
    m_pRenderTarget->DrawBitmap( m_pBitmap );
            
    hr = m_pRenderTarget->EndDraw();

    // Device lost, need to recreate the render target
    // We'll dispose it now and retry drawing
    if ( hr == D2DERR_RECREATE_TARGET )
    {
        hr = S_OK;
        DiscardResources();
    }

    return SUCCEEDED( hr );
}

bool DrawDevice::SaveBMP ( BYTE* Buffer, int width, int height, long paddedsize, LPCTSTR bmpfile )
	{
	// declare bmp structures 
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER info;
	
	// andinitialize them to zero
	memset ( &bmfh, 0, sizeof (BITMAPFILEHEADER ) );
	memset ( &info, 0, sizeof (BITMAPINFOHEADER ) );
	
	// fill the fileheader with data
	bmfh.bfType = 0x4d42;       // 0x4d42 = 'BM'
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + paddedsize;
	bmfh.bfOffBits = 0x36;		// number of bytes to start of bitmap bits
	
	// fill the infoheader

	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = width;
	info.biHeight = height;
	info.biPlanes = 1;			// we only have one bitplane
	info.biBitCount = 24;		// RGB mode is 24 bits
	info.biCompression = BI_RGB;	
	info.biSizeImage = 0;		// can be 0 for 24 bit images
	info.biXPelsPerMeter = 0x0ec4;     // paint and PSP use this values
	info.biYPelsPerMeter = 0x0ec4;     
	info.biClrUsed = 0;			// we are in RGB mode and have no palette
	info.biClrImportant = 0;    // all colors are important

	// now we open the file to write to
	HANDLE file = CreateFile ( bmpfile , GENERIC_WRITE, FILE_SHARE_READ,
		 NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( file == NULL )
	{
		CloseHandle ( file );
		return false;
	}
	// write file header
	unsigned long bwritten;
	if ( WriteFile ( file, &bmfh, sizeof ( BITMAPFILEHEADER ), &bwritten, NULL ) == false )
	{	
		CloseHandle ( file );
		return false;
	}
	// write infoheader
	if ( WriteFile ( file, &info, sizeof ( BITMAPINFOHEADER ), &bwritten, NULL ) == false )
	{	
		CloseHandle ( file );
		return false;
	}
	// write image data
	if ( WriteFile ( file, Buffer, paddedsize, &bwritten, NULL ) == false )
	{	
		CloseHandle ( file );
		return false;
	}
	// and close file
	CloseHandle ( file );
	return true;
	};

BYTE* DrawDevice::ConvertRGBToBMPBuffer ( BYTE* Buffer, int width, int height, long* newsize )
{
	if ( ( NULL == Buffer ) || ( width == 0 ) || ( height == 0 ) )
		return NULL;
	
	int scanlinebytes = width * 3;
	//while ( ( scanlinebytes + padding ) % 5 != 0 ) 
		//padding++;
	padding=0;
	int psw = scanlinebytes + padding;
	*newsize = height * psw;
	BYTE* newbuf = new BYTE[*newsize];
	memset ( newbuf, 0, *newsize );

	long bufpos = 0;   
	long newpos = 0;
	for ( int y = 0; y < height; y++ )
		for ( int x = 0; x < 3 * width; x+=3 )
		{
			bufpos = y * 3 * width + x;     // position in original buffer
			newpos = ( height - y - 1 ) * psw + x; // position in padded buffer
			newbuf[newpos] = 1.164*(Buffer[bufpos]-16)+2.018*(Buffer[bufpos+1]-128);     // swap r and b
			newbuf[newpos + 1] = 1.164*(Buffer[bufpos]-16)-0.813*(Buffer[bufpos+2]-128)-0.391*(Buffer[bufpos+1]-128); // g stays
			newbuf[newpos + 2] = 1.164*(Buffer[bufpos]-16)+1.596*(Buffer[bufpos+2]-128);      // swap b and r
		}
	return newbuf;
}