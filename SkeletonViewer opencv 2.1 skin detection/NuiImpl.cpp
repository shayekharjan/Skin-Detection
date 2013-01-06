//------------------------------------------------------------------------------
// <copyright file="NuiImpl.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// Implementation of CSkeletalViewerApp methods dealing with NUI processing

#include "stdafx.h"
#include "SkeletalViewer.h"
#include "resource.h"
#include <mmsystem.h>
#include <assert.h>
#include <strsafe.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <string.h>
#include "cv.h"
#include "highgui.h"
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <vector>
#include<conio.h>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui/highgui.hpp>
#include <cvblob.h>
//#include <iostream>
using namespace std;
using namespace cv;
//using namespace std;

using namespace cvb;



int headx=0,heady=0,headz=0;

double angleLT=0;
double angleUT=0;
int accessibility;
int var1=0;
int var2=0;
unsigned short int var3=22;
unsigned short int deptharray[240*340];
unsigned short int deptharray2[][240*340];
int lean=0;
int bg=0,cg=0,dg=0;
FILE *accessibilityFile = fopen("Accessibility.txt","a");
double lastdiffticks=0;
double diffticks=0;
IplImage* colorim1 = cvCreateImage(cvSize(640,480), IPL_DEPTH_8U,4);
IplImage* depthim1 = cvCreateImage(cvSize(320,240), 16,1);
IplImage* depthim2 = cvCreateImage(cvSize(320,240), 16,1);
IplImage* colorim2 = cvCreateImage(cvSize(320,240), IPL_DEPTH_8U,4);
IplImage* colorim3 = cvCreateImage(cvSize(320,240), IPL_DEPTH_8U,4);

IplImage *temp0 = cvCreateImage(cvSize(320,240),IPL_DEPTH_8U,1);
IplImage *temp1 = cvCreateImage(cvSize(320,240),IPL_DEPTH_8U,1);
IplImage *temp2 = cvCreateImage(cvSize(320,240),IPL_DEPTH_8U,1);
IplImage *temp3 = cvCreateImage(cvSize(320,240),IPL_DEPTH_8U,1);
IplImage *temp = cvCreateImage(cvSize(320,240),IPL_DEPTH_8U,3);


IplImage *camImage;
IplImage *YCrCb1 = cvCreateImage(cvSize(640,480), IPL_DEPTH_8U,3);
IplImage *YCrCb2 = cvCreateImage(cvSize(320,240), IPL_DEPTH_8U,3);
IplImage *Skin = cvCreateImage(cvSize(320,240), IPL_DEPTH_8U,1);

//IplImage *Skin = cvCreateImage(cvSize(320,240), 8,1);
IplImage *test = cvCreateImage(cvSize(320,240),IPL_DEPTH_8U,3);

clock_t start=clock();

LONG Xarray[20];
LONG Yarray[20];
LONG Darray[20];

CvMat* M1 = cvCreateMat(240,320,CV_16UC1);


static const COLORREF g_JointColorTable[NUI_SKELETON_POSITION_COUNT] = 
{
    RGB(169, 176, 155), // NUI_SKELETON_POSITION_HIP_CENTER
    RGB(169, 176, 155), // NUI_SKELETON_POSITION_SPINE
    RGB(168, 230, 29),  // NUI_SKELETON_POSITION_SHOULDER_CENTER
    RGB(200, 0,   0),   // NUI_SKELETON_POSITION_HEAD
    RGB(79,  84,  33),  // NUI_SKELETON_POSITION_SHOULDER_LEFT
    RGB(84,  33,  42),  // NUI_SKELETON_POSITION_ELBOW_LEFT
    RGB(255, 126, 0),   // NUI_SKELETON_POSITION_WRIST_LEFT
    RGB(215,  86, 0),   // NUI_SKELETON_POSITION_HAND_LEFT
    RGB(33,  79,  84),  // NUI_SKELETON_POSITION_SHOULDER_RIGHT
    RGB(33,  33,  84),  // NUI_SKELETON_POSITION_ELBOW_RIGHT
    RGB(77,  109, 243), // NUI_SKELETON_POSITION_WRIST_RIGHT
    RGB(37,   69, 243), // NUI_SKELETON_POSITION_HAND_RIGHT
    RGB(77,  109, 243), // NUI_SKELETON_POSITION_HIP_LEFT
    RGB(69,  33,  84),  // NUI_SKELETON_POSITION_KNEE_LEFT
    RGB(229, 170, 122), // NUI_SKELETON_POSITION_ANKLE_LEFT
    RGB(255, 126, 0),   // NUI_SKELETON_POSITION_FOOT_LEFT
    RGB(181, 165, 213), // NUI_SKELETON_POSITION_HIP_RIGHT
    RGB(71, 222,  76),  // NUI_SKELETON_POSITION_KNEE_RIGHT
    RGB(245, 228, 156), // NUI_SKELETON_POSITION_ANKLE_RIGHT
    RGB(77,  109, 243)  // NUI_SKELETON_POSITION_FOOT_RIGHT
};

static const COLORREF g_SkeletonColors[NUI_SKELETON_COUNT] =
{
    RGB( 255, 0, 0),
    RGB( 0, 255, 0 ),
    RGB( 64, 255, 255 ),
    RGB( 255, 255, 64 ),
    RGB( 255, 64, 255 ),
    RGB( 128, 128, 255 )
};

//lookups for color tinting based on player index
static const int g_IntensityShiftByPlayerR[] = { 1, 2, 0, 2, 0, 0, 2, 0 };
static const int g_IntensityShiftByPlayerG[] = { 1, 2, 2, 0, 2, 0, 0, 1 };
static const int g_IntensityShiftByPlayerB[] = { 1, 0, 2, 2, 0, 2, 0, 2 };


//-------------------------------------------------------------------
// Nui_Zero
//
// Zero out member variables
//-------------------------------------------------------------------
void CSkeletalViewerApp::Nui_Zero()
{
    if (m_pNuiSensor)
    {
        m_pNuiSensor->Release();
        m_pNuiSensor = NULL;
    }
    m_hNextDepthFrameEvent = NULL;
    m_hNextColorFrameEvent = NULL;
    m_hNextSkeletonEvent = NULL;
    m_pDepthStreamHandle = NULL;
    m_pVideoStreamHandle = NULL;
    m_hThNuiProcess = NULL;
    m_hEvNuiProcessStop = NULL;
    ZeroMemory(m_Pen,sizeof(m_Pen));
    m_SkeletonDC = NULL;
    m_SkeletonBMP = NULL;
    m_SkeletonOldObj = NULL;
    m_PensTotal = 6;
    ZeroMemory(m_Points,sizeof(m_Points));
    m_LastSkeletonFoundTime = 0;
    m_bScreenBlanked = false;
    m_DepthFramesTotal = 0;
    m_LastDepthFPStime = 0;
    m_LastDepthFramesTotal = 0;
    m_pDrawDepth = NULL;
    m_pDrawColor = NULL;
    ZeroMemory(m_SkeletonIds,sizeof(m_SkeletonIds));
    ZeroMemory(m_TrackedSkeletonIds,sizeof(m_SkeletonIds));
}

void CALLBACK CSkeletalViewerApp::Nui_StatusProcThunk( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName, void * pUserData )
{
    reinterpret_cast<CSkeletalViewerApp *>(pUserData)->Nui_StatusProc( hrStatus, instanceName, uniqueDeviceName );
}

//-------------------------------------------------------------------
// Nui_StatusProc
//
// Callback to handle Kinect status changes
//-------------------------------------------------------------------
void CALLBACK CSkeletalViewerApp::Nui_StatusProc( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName )
{
    // Update UI
    PostMessageW( m_hWnd, WM_USER_UPDATE_COMBO, 0, 0 );

    if( SUCCEEDED(hrStatus) )
    {
        if ( S_OK == hrStatus )
        {
            if ( m_instanceId && 0 == wcscmp(instanceName, m_instanceId) )
            {
                Nui_Init(m_instanceId);
            }
            else if ( !m_pNuiSensor )
            {
                Nui_Init();
            }
        }
    }
    else
    {
        if ( m_instanceId && 0 == wcscmp(instanceName, m_instanceId) )
        {
            Nui_UnInit();
            Nui_Zero();
        }
    }
}

//-------------------------------------------------------------------
// Nui_Init
//
// Initialize Kinect by instance name
//-------------------------------------------------------------------
HRESULT CSkeletalViewerApp::Nui_Init( OLECHAR *instanceName )
{
    // Generic creation failure
    if ( NULL == instanceName )
    {
        MessageBoxResource( IDS_ERROR_NUICREATE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }

    HRESULT hr = NuiCreateSensorById( instanceName, &m_pNuiSensor );
    
    // Generic creation failure
    if ( FAILED(hr) )
    {
        MessageBoxResource( IDS_ERROR_NUICREATE, MB_OK | MB_ICONHAND );
        return hr;
    }

    SysFreeString(m_instanceId);

    m_instanceId = m_pNuiSensor->NuiDeviceConnectionId();

    return Nui_Init();
}

//-------------------------------------------------------------------
// Nui_Init
//
// Initialize Kinect
//-------------------------------------------------------------------
HRESULT CSkeletalViewerApp::Nui_Init( )
{
    HRESULT  hr;
    RECT     rc;
    bool     result;

    if ( !m_pNuiSensor )
    {
        HRESULT hr = NuiCreateSensorByIndex(0, &m_pNuiSensor);

        if ( FAILED(hr) )
        {
            return hr;
        }

        SysFreeString(m_instanceId);

        m_instanceId = m_pNuiSensor->NuiDeviceConnectionId();
    }

    m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hNextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    GetWindowRect( GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), &rc );  
    HDC hdc = GetDC( GetDlgItem( m_hWnd, IDC_SKELETALVIEW) );
    
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    m_SkeletonBMP = CreateCompatibleBitmap( hdc, width, height );
    m_SkeletonDC = CreateCompatibleDC( hdc );
    
    ReleaseDC(GetDlgItem(m_hWnd,IDC_SKELETALVIEW), hdc );
    m_SkeletonOldObj = SelectObject( m_SkeletonDC, m_SkeletonBMP );

    m_pDrawDepth = new DrawDevice( );
    result = m_pDrawDepth->Initialize( GetDlgItem( m_hWnd, IDC_DEPTHVIEWER ), m_pD2DFactory, 320, 240, 320 * 4 );
    if ( !result )
    {
        MessageBoxResource( IDS_ERROR_DRAWDEVICE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }

    m_pDrawColor = new DrawDevice( );
    result = m_pDrawColor->Initialize( GetDlgItem( m_hWnd, IDC_VIDEOVIEW ), m_pD2DFactory, 640, 480, 640 * 4 );
    if ( !result )
    {
        MessageBoxResource( IDS_ERROR_DRAWDEVICE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }
    
    DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON |  NUI_INITIALIZE_FLAG_USES_COLOR;
    hr = m_pNuiSensor->NuiInitialize( nuiFlags );
    if ( E_NUI_SKELETAL_ENGINE_BUSY == hr )
    {
        nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH |  NUI_INITIALIZE_FLAG_USES_COLOR;
        hr = m_pNuiSensor->NuiInitialize( nuiFlags) ;
    }
  
    if ( FAILED( hr ) )
    {
        if ( E_NUI_DEVICE_IN_USE == hr )
        {
            MessageBoxResource( IDS_ERROR_IN_USE, MB_OK | MB_ICONHAND );
        }
        else
        {
            MessageBoxResource( IDS_ERROR_NUIINIT, MB_OK | MB_ICONHAND );
        }
        return hr;
    }

    if ( HasSkeletalEngine( m_pNuiSensor ) )
    {
        hr = m_pNuiSensor->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0 );
        if( FAILED( hr ) )
        {
            MessageBoxResource( IDS_ERROR_SKELETONTRACKING, MB_OK | MB_ICONHAND );
            return hr;
        }
    }

    hr = m_pNuiSensor->NuiImageStreamOpen(
        NUI_IMAGE_TYPE_COLOR,
        NUI_IMAGE_RESOLUTION_640x480,
        0,
        2,
        m_hNextColorFrameEvent,
        &m_pVideoStreamHandle );

    if ( FAILED( hr ) )
    {
        MessageBoxResource( IDS_ERROR_VIDEOSTREAM, MB_OK | MB_ICONHAND );
        return hr;
    }

    hr = m_pNuiSensor->NuiImageStreamOpen(
        HasSkeletalEngine(m_pNuiSensor) ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
        NUI_IMAGE_RESOLUTION_320x240,
        0,
        2,
        m_hNextDepthFrameEvent,
        &m_pDepthStreamHandle );

    if ( FAILED( hr ) )
    {
        MessageBoxResource(IDS_ERROR_DEPTHSTREAM, MB_OK | MB_ICONHAND);
        return hr;
    }

    // Start the Nui processing thread
    m_hEvNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hThNuiProcess = CreateThread( NULL, 0, Nui_ProcessThread, this, 0, NULL );

    return hr;
}

//-------------------------------------------------------------------
// Nui_UnInit
//
// Uninitialize Kinect
//-------------------------------------------------------------------
void CSkeletalViewerApp::Nui_UnInit( )
{
    SelectObject( m_SkeletonDC, m_SkeletonOldObj );
    DeleteDC( m_SkeletonDC );
    DeleteObject( m_SkeletonBMP );

    if ( NULL != m_Pen[0] )
    {
        for ( int i = 0; i < NUI_SKELETON_COUNT; i++ )
        {
            DeleteObject( m_Pen[i] );
        }
        ZeroMemory( m_Pen, sizeof(m_Pen) );
    }

    if ( NULL != m_hFontSkeletonId )
    {
        DeleteObject( m_hFontSkeletonId );
        m_hFontSkeletonId = NULL;
    }

    // Stop the Nui processing thread
    if ( NULL != m_hEvNuiProcessStop )
    {
        // Signal the thread
        SetEvent(m_hEvNuiProcessStop);

        // Wait for thread to stop
        if ( NULL != m_hThNuiProcess )
        {
            WaitForSingleObject( m_hThNuiProcess, INFINITE );
            CloseHandle( m_hThNuiProcess );
        }
        CloseHandle( m_hEvNuiProcessStop );
    }

    if ( m_pNuiSensor )
    {
        m_pNuiSensor->NuiShutdown( );
    }
    if ( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextSkeletonEvent );
        m_hNextSkeletonEvent = NULL;
    }
    if ( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
    }
    if ( m_hNextColorFrameEvent && ( m_hNextColorFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextColorFrameEvent );
        m_hNextColorFrameEvent = NULL;
    }

    if ( m_pNuiSensor )
    {
        m_pNuiSensor->Release();
        m_pNuiSensor = NULL;
    }

    // clean up graphics
    delete m_pDrawDepth;
    m_pDrawDepth = NULL;

    delete m_pDrawColor;
    m_pDrawColor = NULL; 

}

DWORD WINAPI CSkeletalViewerApp::Nui_ProcessThread(LPVOID pParam)
{
    CSkeletalViewerApp *pthis = (CSkeletalViewerApp *) pParam;
    return pthis->Nui_ProcessThread();
}

//-------------------------------------------------------------------
// Nui_ProcessThread
//
// Thread to handle Kinect processing
//-------------------------------------------------------------------
DWORD WINAPI CSkeletalViewerApp::Nui_ProcessThread()
{
    const int numEvents = 4;
    HANDLE hEvents[numEvents] = { m_hEvNuiProcessStop, m_hNextDepthFrameEvent, m_hNextColorFrameEvent, m_hNextSkeletonEvent };
    int    nEventIdx;
    DWORD  t;

    m_LastDepthFPStime = timeGetTime( );

    //blank the skeleton display on startup
    m_LastSkeletonFoundTime = 0;

    // Main thread loop
    bool continueProcessing = true;
    while ( continueProcessing )
    {
		//GEOFF put stuff HERE!

        // Wait for any of the events to be signalled
        nEventIdx = WaitForMultipleObjects( numEvents, hEvents, FALSE, 100 );

        // Process signal events
        switch ( nEventIdx )
        {
            case WAIT_TIMEOUT:
                continue;

            // If the stop event, stop looping and exit
            case WAIT_OBJECT_0:
                continueProcessing = false;
                continue;

            case WAIT_OBJECT_0 + 1:
                Nui_GotDepthAlert();
                 ++m_DepthFramesTotal;
                break;

            case WAIT_OBJECT_0 + 2:
                Nui_GotColorAlert();
                break;

            case WAIT_OBJECT_0 + 3:
                Nui_GotSkeletonAlert( );
                break;
        }

        // Once per second, display the depth FPS
        t = timeGetTime( );
        if ( (t - m_LastDepthFPStime) > 1000 )
        {
            int fps = ((m_DepthFramesTotal - m_LastDepthFramesTotal) * 1000 + 500) / (t - m_LastDepthFPStime);
            PostMessageW( m_hWnd, WM_USER_UPDATE_FPS, IDC_FPS, accessibility );
			PostMessageW( m_hWnd, WM_USER_UPDATE_FPS, IDC_VAR1, var1 );
			PostMessageW( m_hWnd, WM_USER_UPDATE_FPS, IDC_VAR2, var2 );
			PostMessageW( m_hWnd, WM_USER_UPDATE_FPS, IDC_VAR3, var3 );
            m_LastDepthFramesTotal = m_DepthFramesTotal;
            m_LastDepthFPStime = t;
        }

        // Blank the skeleton panel if we haven't found a skeleton recently
        if ( (t - m_LastSkeletonFoundTime) > 250 )
        {
            if ( !m_bScreenBlanked )
            {
                Nui_BlankSkeletonScreen( GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), true );
                m_bScreenBlanked = true;
            }
        }
    }

    return 0;
}

//-------------------------------------------------------------------
// Nui_GotDepthAlert
//
// Handle new color data
//-------------------------------------------------------------------
void CSkeletalViewerApp::Nui_GotColorAlert( )
{
    NUI_IMAGE_FRAME imageFrame;

    HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame( m_pVideoStreamHandle, 0, &imageFrame );

    if ( FAILED( hr ) )
    {
        return;
    }

    INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
    NUI_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );


    if ( LockedRect.Pitch != 0 )
    {
        m_pDrawColor->Draw( static_cast<BYTE *>(LockedRect.pBits), LockedRect.size );
		
		BYTE * pBuffer = (BYTE*)LockedRect.pBits;

		cvSetData(colorim1, (BYTE*) pBuffer, colorim1->widthStep);
		//cvShowImage("Color Image",colorim1);
		//cvWaitKey(10);


		char* imname =new char [30];
		clock_t end = clock();
		
		diffticks=end-start/1000;
		
		int Decimal =0;
		int Sign = 0;
		_fcvt_s(imname,10,diffticks,0,&Decimal,&Sign);

		strcat(imname,".jpg");

		char* imnamed =new char [30];

		_fcvt_s(imnamed,10,diffticks,0,&Decimal,&Sign);

		strcat(imnamed,"d.PNG");
		

		char* imnamet =new char [30];

		_fcvt_s(imnamet,10,diffticks,0,&Decimal,&Sign);

		strcat(imnamet,"d.txt");

		int d4 = (int) diffticks;

/*

int x = 0 , y = 0;
int Cr = 0, Cb = 0,w=0,h=0, d=0, w3=0, h3=0;
double YCrCb_h, YCrCb_w, xx, yy, zz, xxp, yyp, zzp, P2Drgbx, P2Drgby, P3DCx, P3DCy, idx2, idy2, chng, chng2;

cvCvtColor(colorim1,YCrCb1,CV_BGR2YCrCb);
cvResize( YCrCb1, YCrCb2, CV_INTER_LINEAR);
cvResize( colorim1, colorim2, CV_INTER_LINEAR); 

cvShowImage("Test Color Image 1",colorim2);

//cvShowImage("Test Color Image 3",YCrCb1);

cvShowImage("Test Color Image 3",YCrCb2);


//cvWaitKey(10);

//cvShowImage("Test Color Image 4",YCrCb);

//cvCvtColor(colorim2,YCrCb,CV_BGR2YCrCb);
YCrCb_w = YCrCb2->width, YCrCb_h = YCrCb2->height;
//cvCvtColor(original_img,YCrCb,CV_BGR2YCrCb);
//w3 = depthim1->width, h3 = depthim1->height, d = depthim1->depth;
//cout << "Depth d: " << d << endl;
//cvResize( colorim1, colorim2, CV_INTER_LINEAR); 
//var1 = YCrCb_h;
//var2 = YCrCb_w;

for (y = 0; y < YCrCb_h; y++)
{
    for (x = 0; x < YCrCb_w; x++)
    {
        //Cr= (int)((unsigned char*)(d3_img->imageData + d3_img->widthStep*(y)))[(x)*3+1];
        //Cb =(int)((unsigned char*)(d3_img->imageData + d3_img->widthStep*(y)))[(x)*3+2];
    
        //zz = colorim1->imageData;
        zz = (int)((unsigned char*)(depthim1->imageData + depthim1->widthStep*(y)))[(x)*3];
        
        if ( zz != 0 )
        {
            // change x and y to cartesian
            xx = (x - 333) * (zz) * (1/594.7) ;
            yy = (y - 262) * (zz) * (1/593.8) ;
            xx = (x - 167) * (zz) * (1/297.35) ;
            yy = (y - 131) * (zz) * (1/296.9) ;
            
            // calibrate 3D data to color camera coordinates
            xxp=0.99984628826577793*xx+0.0012635359098409581*yy-0.017487233004436643*zz+0.0254;
            yyp=-0.0014779096108364480*xx+0.99992385683542895*yy-0.012251380107679535*zz-0.000744;
            zzp=0.017470421412464927*xx+0.012275341476520762*yy+0.99977202419716948*zz-0.00109;
            //color pixel that cooresponds to current 3D point
            P2Drgbx=(xxp*480*(1/zzp))+335;
            P2Drgby=(yyp*490*(1/zzp))+256.676;

            P2Drgbx=(xxp*310*(1/zzp))+160;
            P2Drgby=(yyp*320*(1/zzp))+140;
            
            P3DCx=floor(P2Drgbx+0.5);
            P3DCy=floor(P2Drgby+0.5);
            
            idy2=480-P3DCy;
            idx2=640-P3DCx;
            
            idy2=P3DCy;
            idx2=P3DCx;
            
            chng=0;
            if ( x < 190 )
            {
                chng=((190-x)*32)/190;
                idx2=floor(idx2 + chng);
            }
            if ( x > 190 )
            {   
                chng=((x-200)*20)/120;
                idx2=floor(idx2-chng);
            }
            
            chng2=0;
            if ( y < 130 )
            {
                chng2=((130-y)*23)/130;
                idy2=floor(idy2 + chng2);
            }
            if ( y > 140 )
            {
                chng2=((y-140)*15)/100;
                idy2=floor(idy2 - chng2);
            }
            
            var1 = zz;


            if ( zz < 2 )
            {
                //q1=q1+1;
                //newdeptharray(y,x,1)=xxp;
                //newdeptharray(y,x,2)=yyp;
                //newdeptharray(y,x,3)=zzp;
               
                //depthimage(y,x)=zzp*400;
                
                //newimage(y,x,1)=origim(idy2,idx2,1);
                //newimage(y,x,2)=origim(idy2,idx2,2);
                //newimage(y,x,3)=origim(idy2,idx2,3);


                Cr= (int)((unsigned char*)(YCrCb2->imageData + YCrCb2->widthStep*(y)))[(x)*3+1];
                Cb =(int)((unsigned char*)(YCrCb2->imageData + YCrCb2->widthStep*(y)))[(x)*3+2];

                if ( (Cr>=140 && Cr<=170 ) && (Cb>=100 && Cb<=135))
                {
                        ((unsigned char*)(Skin->imageData + Skin->widthStep*(y)))[(x)] = 255;
                }
                else
                {
                        ((unsigned char*)(Skin->imageData + Skin->widthStep*(y)))[(x)] = 0;
                }



                //cvShowImage("Test Color Image 5",Skin);


                
                //if YCBCR(idy2,idx2,1)>50 && YCBCR(idy2,idx2,2)>70 && YCBCR(idy2,idx2,2)<135 && YCBCR(idy2,idx2,3)>132 && YCBCR(idy2,idx2,3)<154
                //{    
                //    skinimage(y,x)=1;
//%                  elseif YCBCR2(y,x,1)>220
//%       
                      //skinimage(y,x)=1;
                //}
                //else
               // {
                //     skinimage(y,x)=0;
                //}                
            }    
            //else
            //{    depthimage(y,x)=nan;
             //   newdeptharray(y,x,1)=nan;
             //   newdeptharray(y,x,2)=nan;
              //  newdeptharray(y,x,3)=nan;
            //}
        //else
        //    depthimage(y,x)=nan;
        //    newdeptharray(y,x,1)=nan;
        //    newdeptharray(y,x,2)=nan;
        //    newdeptharray(y,x,3)=nan;
        }
            


    }

}




cvShowImage("Test Color Image 4",Skin);


cvWaitKey(10);




*/



		if (diffticks-lastdiffticks>500)
		{
			fprintf(accessibilityFile,"Accessibility: %i, Upper Trunk: %i ,Lower Trunk: %i, CurrentTime:  %i, Skeleton Coordinates: ",accessibility, bg, cg, d4);
			for (int j=0;j<20;j++){
				fprintf(accessibilityFile,"%i, ",Xarray[j]);
				fprintf(accessibilityFile,"%i, ",Yarray[j]);
				fprintf(accessibilityFile,"%i, ",Darray[j]);
			}
			fprintf(accessibilityFile,"\n");

			cvSaveImage(imname,colorim1); // color image capture
 			lastdiffticks=diffticks;

			cv::Mat Ma(240,320, CV_16UC1, deptharray);
			var3=Ma.at<USHORT>(120,160);
			//cv::imwrite(imnamed,Ma);
			IplImage Ma1 = Ma;
			cvSaveImage(imnamed,&Ma1); // depth image capture


			/*std::ofstream myfile;
			myfile.open (imnamet);
			for (int i=0;i<240;i++){
				for (int j=0;j<320;j++){
						myfile << deptharray[i*320+j]<< " ";
				}
				myfile << "\n";
			}
			myfile.close();*/

		}
		//Sleep(300);

		//IplImage* colorim2 = cvCreateImage(size, IPL_DEPTH_8U,3);
		
		//int bufpos=0;
		//for (int y9=0;y9<colorim1->height;y9++)
		//{
		//	uchar* ptr = (uchar*)(colorim1->imageData+y9*colorim1->widthStep);

		//	for (int x9=0;x9<colorim1->width;x9++)
		//	{
		//		bufpos = y9 * 4 * 480 + x9;
		//		ptr[3*x9+0]=static_cast<BYTE *>(LockedRect.pBits)[bufpos+1];
		//		ptr[3*x9+1]=static_cast<BYTE *>(LockedRect.pBits)[bufpos+2];
		//		ptr[3*x9+2]=static_cast<BYTE *>(LockedRect.pBits)[bufpos+3];
		//		//ptr[3*x9+3]=static_cast<BYTE *>(LockedRect.pBits)[bufpos+0];
		//	}
		//}


		//colorim1->imageData=static_cast<BYTE *>(LockedRect.pBits);
		//IplImage* colorim2 = cvCreateImage(size, IPL_DEPTH_8U,3);
		//cvCvtColor(colorim1,colorim2,CV_YCrCb2RGB);
		//cvSaveImage("image1.bmp",colorim2);
		/*int j2=1;
	
		char num32[10];
		_itoa_s(j2,num32, 10);
		string filename("ctest");
		filename += num32;
		filename += ".bmp";

		const size_t newsize = 100;

		size_t origsize = strlen(filename.c_str()) + 5;
		size_t convertedChars=0;
		wchar_t phrase[newsize];
		mbstowcs_s(&convertedChars, phrase, origsize, filename.c_str(), 100);*/
		//long s2;
		//BYTE* c = m_pDrawColor->ConvertRGBToBMPBuffer (static_cast<BYTE *>(LockedRect.pBits), 640, 480, &s2 );
		//m_pDrawColor->SaveBMP(c, 640, 480, s2, L"image1.bmp");
    }
    else
    {
        OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
    }

    pTexture->UnlockRect( 0 );

    m_pNuiSensor->NuiImageStreamReleaseFrame( m_pVideoStreamHandle, &imageFrame );
}

//-------------------------------------------------------------------
// Nui_GotDepthAlert
//
// Handle new depth data
//-------------------------------------------------------------------
void CSkeletalViewerApp::Nui_GotDepthAlert( )
{
    NUI_IMAGE_FRAME imageFrame;
	
    HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame(
        m_pDepthStreamHandle,
        0,
        &imageFrame );

    if ( FAILED( hr ) )
    {
        return;
    }

    INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
    NUI_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if ( 0 != LockedRect.Pitch )
    {
        DWORD frameWidth, frameHeight;
        
        NuiImageResolutionToSize( imageFrame.eResolution, frameWidth, frameHeight );
        
        // draw the bits to the bitmap
        RGBQUAD * rgbrun = m_rgbWk;
        USHORT * pBufferRun = (USHORT *)LockedRect.pBits;

		USHORT xCenter = frameWidth/2;
		USHORT yCenter = frameHeight/2;
		USHORT m_CenterDistance = pBufferRun[xCenter+yCenter*frameWidth]>>3;

		for (int y=0;y<240;y++)
		{
			for (int x=0;x<320;x++)
			{
				deptharray[y*320+x]=pBufferRun[x+y*frameWidth]>>3;
			}
		}



		

int x = 0 , y = 0;
int Cr = 0, Cb = 0,w=0,h=0, d=0, w3=0, h3=0;
double YCrCb_h, YCrCb_w, xx, yy, zz, xxp, yyp, zzp, P2Drgbx, P2Drgby, P3DCx, P3DCy, idx2, idy2, chng, chng2;

cvCvtColor(colorim1,YCrCb1,CV_BGR2YCrCb);               // converting the color image to YCrCb
cvResize( YCrCb1, YCrCb2, CV_INTER_LINEAR);             // resizing the YCrCb image to 320x240 
cvResize( colorim1, colorim2, CV_INTER_LINEAR);         // resizing the color image to 320x240

//cvShowImage("Test Color Image 1",colorim1);
//cvShowImage("Test Color Image 2",colorim2);

//cvShowImage("Test Color Image 3",YCrCb1);

//cvShowImage("Test Color Image 4",YCrCb2);


//cvWaitKey(10);

//cvShowImage("Test Color Image 4",YCrCb);

//cvCvtColor(colorim2,YCrCb,CV_BGR2YCrCb);
YCrCb_w = YCrCb2->width, YCrCb_h = YCrCb2->height;
//cvCvtColor(original_img,YCrCb,CV_BGR2YCrCb);
//w3 = depthim1->width, h3 = depthim1->height, d = depthim1->depth;
//cout << "Depth d: " << d << endl;
//cvResize( colorim1, colorim2, CV_INTER_LINEAR); 
//var1 = YCrCb_h;
//var2 = YCrCb_w;
cv::Mat Ma(240,320, CV_16UC1, depthim1);                // creating a new depth matrix which has 16bit with channel 1

// making the depth matrix to be black all the time

for ( int m = 0; m < Skin->height; m++ )
			{
				for ( int n = 0; n < Skin->width; n++ )
				{
					((unsigned char*)(Skin->imageData + Skin->widthStep*(m)))[(n)] = 0;
				}
			}


// going through the entire YCrCb image pixel by pixel


for (y = 1; y < YCrCb_h; y++)
{
    for (x = 1; x < YCrCb_w; x++)
    {
        //Cr= (int)((unsigned char*)(d3_img->imageData + d3_img->widthStep*(y)))[(x)*3+1];
        //Cb =(int)((unsigned char*)(d3_img->imageData + d3_img->widthStep*(y)))[(x)*3+2];
    
        //zz = colorim1->imageData;
        //zz = (int)((unsigned char*)(depthim1->imageData + depthim1->widthStep*(120)))[(100)*3];
        zz = pBufferRun[x+y*frameWidth]>>3;            // getting the depth of the image
		
		//zz =  (Ma.at<USHORT>(120,160));

		var1 = zz;

        if ( zz != 0 )
        {
            // change x and y to cartesian
            xx = (x - 333) * (zz) * (1/594.7) ;
            yy = (y - 262) * (zz) * (1/593.8) ;
            xx = (x - 167) * (zz) * (1/297.35) ;
            yy = (y - 131) * (zz) * (1/296.9) ;
            
            // calibrate 3D data to color camera coordinates
            xxp=0.99984628826577793*xx+0.0012635359098409581*yy-0.017487233004436643*zz+0.0254;
            yyp=-0.0014779096108364480*xx+0.99992385683542895*yy-0.012251380107679535*zz-0.000744;
            zzp=0.017470421412464927*xx+0.012275341476520762*yy+0.99977202419716948*zz-0.00109;
            //color pixel that cooresponds to current 3D point
            P2Drgbx=(xxp*480*(1/zzp))+335;
            P2Drgby=(yyp*490*(1/zzp))+256.676;

            P2Drgbx=(xxp*310*(1/zzp))+160;
            P2Drgby=(yyp*320*(1/zzp))+140;
            
            P3DCx=floor(P2Drgbx+0.5);
            P3DCy=floor(P2Drgby+0.5);
            
            idy2=480-P3DCy;
            idx2=640-P3DCx;
            
            idy2=P3DCy;
            idx2=P3DCx;
            
            chng=0;
            if ( x < 190 )
            {
                chng=((190-x)*32)/190;
                idx2=floor(idx2 + chng);
            }
            if ( x > 190 )
            {   
                chng=((x-200)*20)/120;
                idx2=floor(idx2-chng);
            }
            
            chng2=0;
            if ( y < 130 )
            {
                chng2=((130-y)*23)/130;
                idy2=floor(idy2 + chng2);
            }
            if ( y > 140 )
            {
                chng2=((y-140)*15)/100;
                idy2=floor(idy2 - chng2);
            }
            
           // var1 = zz;
			//var2 = zz;


			


            if ( zz <= 1000 )                           // comparing the depth of the image
            {
                //q1=q1+1;
                //newdeptharray(y,x,1)=xxp;
                //newdeptharray(y,x,2)=yyp;
                //newdeptharray(y,x,3)=zzp;
				 //deptharray2[x,y, 1] = xxp;
				 //deptharray2[x,y, 2] = yyp;
				 //deptharray2[x,y, 3] = zzp;
				//deptharray2[x][y] = xxp;
				//deptharray2[x][y+1] = yyp;
				//deptharray2[x][y+2] = zzp;

                //depthimage(y,x)=zzp*400;
                 //depthim2[x] = zzp*400;
                //newimage(y,x,1)=origim(idy2,idx2,1);
                //newimage(y,x,2)=origim(idy2,idx2,2);
                //newimage(y,x,3)=origim(idy2,idx2,3);
				
				//colorim3[x,y,1] = colorim2[idy2,idx2,1];
				//colorim3[x,y,2] = colorim2[idy2,idx2,2];
				//colorim3[x,y,3] = colorim2[idy2,idx2,3];

				//((unsigned char*)(colorim3->imageData + colorim3->widthStep*(y)))[(x)*3] = ((unsigned char*)(colorim2->imageData + colorim2->widthStep*((int)idy2)))[(int)(idx2)*3];

				//((unsigned char*)(colorim3->imageData + colorim3->widthStep*(y)))[(x)*3+1] = ((unsigned char*)(colorim2->imageData + colorim2->widthStep*((int)idy2)))[(int)(idx2)*3+1];

//				((unsigned char*)(colorim3->imageData + colorim3->widthStep*(y)))[(x)*3+2] = ((unsigned char*)(colorim2->imageData + colorim2->widthStep*((int)idy2)))[(int)(idx2)*3+2];

                Cr= (int)((unsigned char*)(YCrCb2->imageData + YCrCb2->widthStep*(y)))[(x)*3+1];
                Cb =(int)((unsigned char*)(YCrCb2->imageData + YCrCb2->widthStep*(y)))[(x)*3+2];

                if ( (Cr>=140 && Cr<=170 ) && (Cb>=100 && Cb<=135))
                {
                    //((unsigned char*)(Skin->imageData + Skin->widthStep*(y)))[(x)] = 255;
					((unsigned char*)(Skin->imageData + Skin->widthStep*((int)idy2)))[(int)(idx2)] = 255;       // making the variable to be integers and assigning white to the respective points on the skin image
						//deptharray[y*320+x]=pBufferRun[x+y*frameWidth]>>3;
                }
                else
                {
                    // ((unsigned char*)(Skin->imageData + Skin->widthStep*((int)idy2)))[(int)(idx2)] = 0;

                }


				//for (int y=0;y<240;y++)
		       // {
			    //  for (int x=0;x<320;x++)
			     // {
				 //   deptharray[y*320+x]=pBufferRun[x+y*frameWidth]>>3;
			     // }
		      // }







                //cvShowImage("Test Color Image 5",Skin);


                
                //if YCBCR(idy2,idx2,1)>50 && YCBCR(idy2,idx2,2)>70 && YCBCR(idy2,idx2,2)<135 && YCBCR(idy2,idx2,3)>132 && YCBCR(idy2,idx2,3)<154
                //{    
                //    skinimage(y,x)=1;
//%                  elseif YCBCR2(y,x,1)>220
//%       
                      //skinimage(y,x)=1;
                //}
                //else
               // {
                //     skinimage(y,x)=0;
                //}                
            }    
            //else
            //{    depthimage(y,x)=nan;
             //   newdeptharray(y,x,1)=nan;
             //   newdeptharray(y,x,2)=nan;
              //  newdeptharray(y,x,3)=nan;
            //}
			else
			{
				//deptharray2[x,y,1] = 0;
				//deptharray2[x,y,2] = 0;
				//deptharray2[x,y,3] = 0;
				//((unsigned char*)(Skin->imageData + Skin->widthStep*((int)idy2)))[(int)(idx2)] = 0;
				//((unsigned char*)(Skin->imageData + Skin->widthStep*(y)))[(x)] = 0;
				//deptharray2[x][y] = 0;
				//deptharray2[x][y+1] = 0;
				//deptharray2[x][y+2] = 0;
			}
        //else
        //    depthimage(y,x)=nan;
        //    newdeptharray(y,x,1)=nan;
        //    newdeptharray(y,x,2)=nan;
        //    newdeptharray(y,x,3)=nan;
        }
            


    }

}


//cvSmooth(Skin, Skin, CV_MEDIAN, 5, 0, 0, 0);

//
cvDilate(Skin,Skin, 0, 1);
cvSmooth(Skin, Skin, CV_MEDIAN, 3, 0, 0, 0);
//cvSmooth(Skin, Skin, CV_GAUSSIAN, 3, 0, 0, 0);




cvShowImage("Test Color Image Skin",Skin);

cvSaveImage("testskin.jpg", Skin);

for ( int m = 0; m < Skin->height; m++ )
			{
				for ( int n = 0; n < Skin->width; n++ )
				{
					((unsigned char*)(Skin->imageData + Skin->widthStep*(m)))[(n)] = 0;
				}
			}

		
//cvShowImage("Testttt", depthim1);
//cvShowImage("Testt", depthim2);

//cvShowImage("Test ColorIm3 Image ",colorim3);

cvWaitKey(10);







        // end pixel is start + width*height - 1
        USHORT * pBufferEnd = pBufferRun + (frameWidth * frameHeight);

        assert( frameWidth * frameHeight <= ARRAYSIZE(m_rgbWk) );

        while ( pBufferRun < pBufferEnd )
        {
            *rgbrun = Nui_ShortToQuad_Depth( *pBufferRun );
            ++pBufferRun;
            ++rgbrun;
        }

        m_pDrawDepth->Draw( (BYTE*) m_rgbWk, frameWidth * frameHeight * 4 );
		var3 = m_CenterDistance;
    }
    else
    {
        OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
    }

    pTexture->UnlockRect( 0 );

    m_pNuiSensor->NuiImageStreamReleaseFrame( m_pDepthStreamHandle, &imageFrame );
	
}

void CSkeletalViewerApp::Nui_BlankSkeletonScreen (HWND hWnd, bool getDC )
{
    HDC hdc = getDC ? GetDC( hWnd ) : m_SkeletonDC;

    RECT rct;
    GetClientRect( hWnd, &rct );
    PatBlt( hdc, 0, 0, rct.right, rct.bottom, BLACKNESS );

    if ( getDC )
    {
        ReleaseDC( hWnd, hdc );
    }
}

void CSkeletalViewerApp::Nui_DrawSkeletonSegment( NUI_SKELETON_DATA * pSkel, int numJoints, ... )
{
    va_list vl;
    va_start(vl,numJoints);

    POINT segmentPositions[NUI_SKELETON_POSITION_COUNT];
    int segmentPositionsCount = 0;

    DWORD polylinePointCounts[NUI_SKELETON_POSITION_COUNT];
    int numPolylines = 0;
    int currentPointCount = 0;

    // Note the loop condition: We intentionally run one iteration beyond the
    // last element in the joint list, so we can properly end the final polyline.
    for ( int iJoint = 0; iJoint <= numJoints; iJoint++ )
    {
        if ( iJoint < numJoints )
        {
            NUI_SKELETON_POSITION_INDEX jointIndex = va_arg( vl, NUI_SKELETON_POSITION_INDEX );

            if ( pSkel->eSkeletonPositionTrackingState[jointIndex] != NUI_SKELETON_POSITION_NOT_TRACKED )
            {
                // This joint is tracked: add it to the array of segment positions.            
                segmentPositions[segmentPositionsCount] = m_Points[jointIndex];
                segmentPositionsCount++;
                currentPointCount++;

                // Fully processed the current joint; move on to the next one
                continue;
            }
        }

        // If we fall through to here, we're either beyond the last joint, or
        // the current joint is not tracked: end the current polyline here.
        if ( currentPointCount > 1 )
        {
            // Current polyline already has at least two points: save the count.
            polylinePointCounts[numPolylines++] = currentPointCount;
        }
        else if ( currentPointCount == 1 )
        {
            // Current polyline has only one point: ignore it.
            segmentPositionsCount--;
        }
        currentPointCount = 0;
    }

#ifdef _DEBUG
    // We should end up with no more points in segmentPositions than the
    // original number of joints.
    assert(segmentPositionsCount <= numJoints);

    int totalPointCount = 0;
    for (int i = 0; i < numPolylines; i++)
    {
        // Each polyline should contain at least two points.
        assert(polylinePointCounts[i] > 1);

        totalPointCount += polylinePointCounts[i];
    }

    // Total number of points in all polylines should be the same as number
    // of points in segmentPositions.
    assert(totalPointCount == segmentPositionsCount);
#endif

    if (numPolylines > 0)
    {
        PolyPolyline( m_SkeletonDC, segmentPositions, polylinePointCounts, numPolylines );
    }

    va_end(vl);
}

void CSkeletalViewerApp::Nui_DrawSkeleton( NUI_SKELETON_DATA * pSkel, HWND hWnd, int WhichSkeletonColor )
{
    HGDIOBJ hOldObj = SelectObject( m_SkeletonDC, m_Pen[WhichSkeletonColor % m_PensTotal] );
    
    RECT rct;
    GetClientRect(hWnd, &rct);
    int width = rct.right;
    int height = rct.bottom;
    
    if ( m_Pen[0] == NULL )
    {
        for (int i = 0; i < m_PensTotal; i++)
        {
            m_Pen[i] = CreatePen( PS_SOLID, width / 80, g_SkeletonColors[i] );
        }
    }

    int i;
    USHORT depth;
    LONG x1=0,x2=0,x3=0,y1=0,y2=0,y3=0,z1=0,z2=0,z3=0;
	LONG x21=0,x22=0,x23=0,y21=0,y22=0,y23=0,z21=0,z22=0,z23=0,x31=0,z31=0,zf;
	double n1=0,n2=0,n3=0;
	double n21=0,n22=0,n23=0;


	for (i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
    {
        NuiTransformSkeletonToDepthImage( pSkel->SkeletonPositions[i], &m_Points[i].x, &m_Points[i].y, &depth );
		Xarray[i]=m_Points[i].x;
		Yarray[i]=m_Points[i].y;
		//Darray[i]=0.0115*depth-1;
	}

    for (i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
    {
        NuiTransformSkeletonToDepthImage( pSkel->SkeletonPositions[i], &m_Points[i].x, &m_Points[i].y, &depth );

		zf = 0.0115*depth-1;

		Darray[i] = zf;

		if( i==0 )
		{
			x1 = m_Points[i].x;
			y1 = m_Points[i].y;
			z1 = 0.0115*depth-1;
		}
		if( i==12 )
		{
			x2 = m_Points[i].x;
			y2 = m_Points[i].y;
			z2 = 0.0115*depth-1;
		}
		if( i==16 )
		{
			x3 = m_Points[i].x;
			y3 = m_Points[i].y;
			z3 = 0.0115*depth-1;
		}
		if( i==1 )
		{
			x21 = m_Points[i].x;
			y21 = m_Points[i].y;
			z21 = 0.0115*depth-1;
		}
		if( i==2 )
		{
			x31=m_Points[i].x;
			z31=0.0115*depth-1;
		}
		if( i==4 )
		{
			x22 = m_Points[i].x;
			y22 = m_Points[i].y;
			z22 = 0.0115*depth-1;
		}
		if( i==8 )
		{
			x23 = m_Points[i].x;
			y23 = m_Points[i].y;
			z23 = 0.0115*depth-1;
		}

		if( i==3 )
		{
			headx = (int) m_Points[i].x;
			heady = (int) m_Points[i].y;
			headz = (int) 0.0115*depth-1; //z in cm. 
		}


        m_Points[i].x = (m_Points[i].x * width) / 320;
        m_Points[i].y = (m_Points[i].y * height) / 240;

    }
	
	n1=(y2-y1)*(z3-z1)-(y3-y1)*(z2-z1);
	n2=(x3-x1)*(z2-z1)-(x2-x1)*(z3-z1);
	n3=(x2-x1)*(y3-y1)-(x3-x1)*(y2-y1);

	//int angleLT=0;

	angleLT = 180-180*acos ( n3 / (sqrt(n1*n1+n2*n2+n3*n3)))/3.14159;

	n21=(y22-y21)*(z23-z21)-(y23-y21)*(z22-z21);
	n22=(x23-x21)*(z22-z21)-(x22-x21)*(z23-z21);
	n23=(x22-x21)*(y23-y21)-(x23-x21)*(y22-y21);

	//int angleUT=0;

	angleUT = 180*acos ( n23 / (sqrt(n21*n21+n22*n22+n23*n23)))/3.14159;


	int minangle=10;
	int maxangle=20;
	int leanthreshold=50;

	if ((x1-x31)*(x1-x31)<leanthreshold) lean=0;
	if ((x1-x31)*(x1-x31)>leanthreshold) lean=1;

	if ((angleLT<minangle && angleUT<minangle) || (angleLT<minangle && angleUT<maxangle && angleUT>minangle && lean==0) || (angleUT<minangle && angleLT<maxangle && angleLT>minangle && lean==0)) accessibility=4;
	if ((angleLT<minangle && angleUT<maxangle && angleUT>minangle && lean==1) || (angleUT<minangle && angleLT<maxangle && angleLT>minangle && lean==1)) accessibility=3;
	if ((angleLT>minangle && angleLT<maxangle && angleUT>minangle && angleUT<maxangle) || (angleLT>minangle && angleLT<maxangle && angleUT>maxangle) || (angleLT>maxangle && angleUT>minangle && angleUT<maxangle) || (angleLT>maxangle && angleUT<minangle) || (angleLT<minangle && angleUT>maxangle)) accessibility=2;
	if (angleLT>maxangle && angleUT>maxangle) accessibility=1;

	int b = (int) angleUT;
	int c = (int) angleLT;
	int d = (int) diffticks;
	//accessibility = b;
	var1=angleUT;
	var2=angleLT;
	//var3=lean;

	bg=b;
	cg=c;
	dg=d;


	//fprintf(accessibilityFile,"Accessibility: %i Upper Trunk: %i  Lower Trunk %i  CurrentTime:  %i\n",accessibility, b, c, d);




    SelectObject(m_SkeletonDC,m_Pen[WhichSkeletonColor%m_PensTotal]);
    
    Nui_DrawSkeletonSegment(pSkel,4,NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_HEAD);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);
    
    // Draw the joints in a different color
    for ( i = 0; i < NUI_SKELETON_POSITION_COUNT ; i++ )
    {
        if ( pSkel->eSkeletonPositionTrackingState[i] != NUI_SKELETON_POSITION_NOT_TRACKED )
        {
            HPEN hJointPen;
        
            hJointPen = CreatePen( PS_SOLID, 9, g_JointColorTable[i] );
            hOldObj = SelectObject( m_SkeletonDC, hJointPen );

            MoveToEx( m_SkeletonDC, m_Points[i].x, m_Points[i].y, NULL );
            LineTo( m_SkeletonDC, m_Points[i].x, m_Points[i].y );

            SelectObject( m_SkeletonDC, hOldObj );
            DeleteObject( hJointPen );
        }
    }

    if (m_bAppTracking)
    {
        Nui_DrawSkeletonId(pSkel, hWnd, WhichSkeletonColor);
    }
}

void CSkeletalViewerApp::Nui_DrawSkeletonId( NUI_SKELETON_DATA * pSkel, HWND hWnd, int WhichSkeletonColor )
{
    RECT rct;
    GetClientRect( hWnd, &rct );

    float fx = 0, fy = 0;

    NuiTransformSkeletonToDepthImage( pSkel->Position, &fx, &fy );

    int skelPosX = (int)( fx * rct.right + 0.5f );
    int skelPosY = (int)( fy * rct.bottom + 0.5f );

    WCHAR number[20];
    size_t length;

    if ( FAILED(StringCchPrintf(number, ARRAYSIZE(number), L"%d", pSkel->dwTrackingID)) )
    {
        return;
    }

    if ( FAILED(StringCchLength(number, ARRAYSIZE(number), &length)) )
    {
        return;
    }

    if ( m_hFontSkeletonId == NULL )
    {
        LOGFONT lf;
        GetObject( (HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf );
        lf.lfHeight *= 2;
        m_hFontSkeletonId = CreateFontIndirect( &lf );
    }

    HGDIOBJ hLastFont = SelectObject( m_SkeletonDC, m_hFontSkeletonId );
    SetTextAlign( m_SkeletonDC, TA_CENTER );
    SetTextColor( m_SkeletonDC, g_SkeletonColors[WhichSkeletonColor] );
    SetBkColor( m_SkeletonDC, RGB(0, 0, 0) );

    TextOut( m_SkeletonDC, skelPosX, skelPosY, number, (int)length);

    SelectObject( m_SkeletonDC, hLastFont );
}

void CSkeletalViewerApp::Nui_DoDoubleBuffer( HWND hWnd, HDC hDC )
{
    RECT rct;
    GetClientRect(hWnd, &rct);

    HDC hdc = GetDC( hWnd );

    BitBlt( hdc, 0, 0, rct.right, rct.bottom, hDC, 0, 0, SRCCOPY );

    ReleaseDC( hWnd, hdc );
}

//-------------------------------------------------------------------
// Nui_GotSkeletonAlert
//
// Handle new skeleton data
//-------------------------------------------------------------------
void CSkeletalViewerApp::Nui_GotSkeletonAlert( )
{
    NUI_SKELETON_FRAME SkeletonFrame = {0};

    bool bFoundSkeleton = false;

    if ( SUCCEEDED(m_pNuiSensor->NuiSkeletonGetNextFrame( 0, &SkeletonFrame )) )
    {
        for ( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
        {
            if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED ||
                (SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_POSITION_ONLY && m_bAppTracking))
            {
                bFoundSkeleton = true;
            }
        }
    }

    // no skeletons!
    if( !bFoundSkeleton )
    {
        return;
    }

    // smooth out the skeleton data
    HRESULT hr = m_pNuiSensor->NuiTransformSmooth(&SkeletonFrame,NULL);
    if ( FAILED(hr) )
    {
        return;
    }

    // we found a skeleton, re-start the skeletal timer
    m_bScreenBlanked = false;
    m_LastSkeletonFoundTime = timeGetTime( );

    // draw each skeleton color according to the slot within they are found.
    Nui_BlankSkeletonScreen( GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), false );

    bool bSkeletonIdsChanged = false;
    for ( int i = 0 ; i < NUI_SKELETON_COUNT; i++ )
    {
        if ( m_SkeletonIds[i] != SkeletonFrame.SkeletonData[i].dwTrackingID )
        {
            m_SkeletonIds[i] = SkeletonFrame.SkeletonData[i].dwTrackingID;
            bSkeletonIdsChanged = true;
        }

        // Show skeleton only if it is tracked, and the center-shoulder joint is at least inferred.
        if ( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&
            SkeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER] != NUI_SKELETON_POSITION_NOT_TRACKED)
        {
            Nui_DrawSkeleton( &SkeletonFrame.SkeletonData[i], GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), i );
        }
        else if ( m_bAppTracking && SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_POSITION_ONLY )
        {
            Nui_DrawSkeletonId( &SkeletonFrame.SkeletonData[i], GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), i );
        }
    }

    if ( bSkeletonIdsChanged )
    {
        UpdateTrackingComboBoxes();
    }

    Nui_DoDoubleBuffer(GetDlgItem(m_hWnd,IDC_SKELETALVIEW), m_SkeletonDC);
}

//-------------------------------------------------------------------
// Nui_ShortToQuad_Depth
//
// Get the player colored depth value
//-------------------------------------------------------------------
RGBQUAD CSkeletalViewerApp::Nui_ShortToQuad_Depth( USHORT s )
{
    USHORT RealDepth = NuiDepthPixelToDepth(s);
    USHORT Player    = NuiDepthPixelToPlayerIndex(s);

    // transform 13-bit depth information into an 8-bit intensity appropriate
    // for display (we disregard information in most significant bit)
    BYTE intensity = (BYTE)~(RealDepth >> 4);

    // tint the intensity by dividing by per-player values
    RGBQUAD color;
    color.rgbRed   = intensity >> g_IntensityShiftByPlayerR[Player];
    color.rgbGreen = intensity >> g_IntensityShiftByPlayerG[Player];
    color.rgbBlue  = intensity >> g_IntensityShiftByPlayerB[Player];

    return color;
}

void CSkeletalViewerApp::Nui_SetApplicationTracking(bool applicationTracks)
{
    if ( HasSkeletalEngine(m_pNuiSensor) )
    {
        HRESULT hr = m_pNuiSensor->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, applicationTracks ? NUI_SKELETON_TRACKING_FLAG_TITLE_SETS_TRACKED_SKELETONS : 0);
        if ( FAILED( hr ) )
        {
            MessageBoxResource(IDS_ERROR_SKELETONTRACKING, MB_OK | MB_ICONHAND);
        }
    }
}

void CSkeletalViewerApp::Nui_SetTrackedSkeletons(int skel1, int skel2)
{
    m_TrackedSkeletonIds[0] = skel1;
    m_TrackedSkeletonIds[1] = skel2;
    DWORD tracked[NUI_SKELETON_MAX_TRACKED_COUNT] = { skel1, skel2 };
    if ( FAILED(m_pNuiSensor->NuiSkeletonSetTrackedSkeletons(tracked)) )
    {
        MessageBoxResource(IDS_ERROR_SETTRACKED, MB_OK | MB_ICONHAND);
    }
}