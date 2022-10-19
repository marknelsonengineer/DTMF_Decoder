///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder22X!Vqrpp1kz9C!ma3mCkbd - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
/// 
/// @file audio.cpp
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <strsafe.h>
#include <AudioClient.h>

#include "audio.h"
#include "mvcModel.h"

#include <assert.h>  // For assert
#include <avrt.h>    // For AvSetMmThreadCharacteristics()

#pragma comment(lib, "avrt")    // Link the MMCSS library


IMMDevice*      gDevice = NULL;
LPWSTR          glpwstrDeviceId = NULL;
DWORD           glpdwState = 0;
IPropertyStore* glpPropertyStore = NULL;
PROPVARIANT     DeviceInterfaceFriendlyName;  // Container for the friendly name of the audio adapter for the device
PROPVARIANT     DeviceDescription;            // Container for the device's description
PROPVARIANT     DeviceFriendlyName;           // Container for friendly name of the device
IAudioClient*   glpAudioClient = NULL;
REFERENCE_TIME  gDefaultDevicePeriod = -1;    // Expressed in 100ns units (dev machine = 101,587 = 10.1587ms)
REFERENCE_TIME  gMinimumDevicePeriod = -1;    // Expressed in 100ns units (dev machine = 29,025  = 2.9025ms
BOOL            gExclusiveAudioMode = false;
UINT32          guBufferSize = 0;             // Dev Machine = 182
HANDLE          gAudioSamplesReadyEvent = NULL;;
HANDLE          hCaptureThread = NULL;
IAudioCaptureClient* gCaptureClient = NULL;


template <class T> void SafeRelease( T** ppT ) {
   if ( *ppT ) {
      ( *ppT )->Release();
      *ppT = NULL;
   }
}


DWORD captureThread( LPVOID Context ) {
   OutputDebugStringA( __FUNCTION__ ":  Start capture thread" );
   HRESULT hr;
   HANDLE mmcssHandle = NULL;
   DWORD mmcssTaskIndex = 0;

   /// Initialize COM for the thread
   hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to initialize COM in thread" );
      ExitThread( -1 );
   }

   /// Set the multimedia class scheduler service, which will set the CPU
   /// priority for this thread
   mmcssHandle = AvSetMmThreadCharacteristics( L"Capture", &mmcssTaskIndex );
   if ( mmcssHandle == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to set MMCSS on thread.  Continuing." );
   }

   /// Audio capture loop
   /// isRunning gets set to false by WM_CLOSE
   
   isRunning = true;

   while ( isRunning ) {
   }

   /// Cleanup the thread
   CoUninitialize();

   OutputDebugStringA( __FUNCTION__ ":  End capture thread" );

   ExitThread( 0 );
}


BOOL initAudioDevice( HWND hWnd ) {
   /// Get IMMDeviceEnumerator from COM (CoCreateInstance)

   IMMDeviceEnumerator* deviceEnumerator = NULL;
   HRESULT hr;

   hr = CoCreateInstance( __uuidof( MMDeviceEnumerator ), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &deviceEnumerator ) );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to instantiate the multimedia device enumerator via COM" );
      return FALSE;
   }

   /// Get the IMMDevice
   assert( gDevice == NULL );

   hr = deviceEnumerator->GetDefaultAudioEndpoint( eCapture, eMultimedia, &gDevice );
   if ( hr != S_OK || gDevice == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get default audio device" );
      return FALSE;
   }

   /// Get the ID from IMMDevice
   hr = gDevice->GetId( &glpwstrDeviceId );
   if ( hr != S_OK || glpwstrDeviceId == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get the device's ID string" );
      return FALSE;
   }

   OutputDebugStringW( __FUNCTIONW__ L":  Device ID follows" );
   OutputDebugStringW( glpwstrDeviceId );

   /// Get the State from IMMDevice
   hr = gDevice->GetState( &glpdwState );
   if ( hr != S_OK || glpdwState == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get the device's state" );
      return FALSE;
   }

   if ( glpdwState != DEVICE_STATE_ACTIVE ) {
      OutputDebugStringA( __FUNCTION__ ":  The audio device state is not active" );
      return FALSE;
   }

   /// Get the Property Store from IMMDevice
   hr = gDevice->OpenPropertyStore( STGM_READ, &glpPropertyStore );
   if ( hr != S_OK || glpPropertyStore == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to open device property store" );
      return FALSE;
   }

   /// Get the device's properties from the property store
   PropVariantInit( &DeviceInterfaceFriendlyName );     /// Get the friendly name of the audio adapter for the device
   PropVariantInit( &DeviceDescription );               /// Get the device's description
   PropVariantInit( &DeviceFriendlyName );              /// Get the friendly name of the device

   hr = glpPropertyStore->GetValue( PKEY_DeviceInterface_FriendlyName, &DeviceInterfaceFriendlyName );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to retrieve the friendly name of the audio adapter for the device.  Continuing." );
      DeviceInterfaceFriendlyName.pcVal = NULL;
   } else {
      OutputDebugStringA( __FUNCTION__ ":  The friendly name of the audio adapter for the device follows" );
      OutputDebugStringW( DeviceInterfaceFriendlyName.pwszVal );
   }

   hr = glpPropertyStore->GetValue( PKEY_Device_DeviceDesc, &DeviceDescription );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to retrieve the device's description.  Continuing." );
      DeviceDescription.pcVal = NULL;
   } else {
      OutputDebugStringA( __FUNCTION__ ":  The device's description follows" );
      OutputDebugStringW( DeviceDescription.pwszVal );
   }

   hr = glpPropertyStore->GetValue( PKEY_Device_FriendlyName, &DeviceFriendlyName );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to retrieve the friendly name of the device.  Continuing." );
      DeviceFriendlyName.pcVal = NULL;
   } else {
      OutputDebugStringA( __FUNCTION__ ":  The friendly name of the device follows" );
      OutputDebugStringW( DeviceFriendlyName.pwszVal );
   }

   /// Use Activate on IMMDevice to create an IAudioClient
   hr = gDevice->Activate( __uuidof( IAudioClient ), CLSCTX_ALL, NULL, (void**) &glpAudioClient );
   if ( hr != S_OK || glpAudioClient == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to create an audio client" );
      return FALSE;
   }

   /// Get the device period
   hr = glpAudioClient->GetDevicePeriod( &gDefaultDevicePeriod, &gMinimumDevicePeriod );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get audio client device periods" );
      return FALSE;
   }

   /// See if the audio device supports the format we want
   WAVEFORMATEX tryThisFormat;
   tryThisFormat.wFormatTag = WAVE_FORMAT_PCM;
   tryThisFormat.nChannels = 1;
   tryThisFormat.nSamplesPerSec = 8000;
   tryThisFormat.nAvgBytesPerSec = 8000;
   tryThisFormat.nBlockAlign = 1;
   tryThisFormat.wBitsPerSample = 8;
   tryThisFormat.cbSize = 0;

   hr = glpAudioClient->IsFormatSupported( AUDCLNT_SHAREMODE_EXCLUSIVE, &tryThisFormat, NULL );
   if ( hr == S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  The requested format is supported in exclusive mode" );
      gExclusiveAudioMode = true;
   } else if ( hr == AUDCLNT_E_UNSUPPORTED_FORMAT ) {
      OutputDebugStringA( __FUNCTION__ ":  The requested format is supported in shared mode" );
      gExclusiveAudioMode = false;
   } else {
      OutputDebugStringA( __FUNCTION__ ":  Failed to check the requested format" );
      return FALSE;
   }


   if ( gExclusiveAudioMode ) {
      OutputDebugStringA( __FUNCTION__ ":  Exclusive mode not supported right now... using shared mode until theres a problem" );
      gExclusiveAudioMode = false;
   }

   /// Initialize shared mode audio client
   //  Shared mode streams using event-driven buffering must set both periodicity and bufferDuration to 0.
   hr = glpAudioClient->Initialize( AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK
                                                            | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
                                                                                                , 0, 0, &tryThisFormat, NULL );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to initialize the audio client in shared mode" );
      return FALSE;
   }

   OutputDebugStringA( __FUNCTION__ ":  The audio client has been initialized in shared mode" );

   hr = glpAudioClient->GetBufferSize( &guBufferSize );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get buffer size" );
      return FALSE;
   }

   /// Create the callback events
   gAudioSamplesReadyEvent = CreateEventEx( NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE );
   if ( gAudioSamplesReadyEvent == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to create audio samples ready event" );
      return FALSE;
   }

   hr = glpAudioClient->SetEventHandle( gAudioSamplesReadyEvent );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to set audio capture ready event" );
      return FALSE;
   }

   hr = glpAudioClient->GetService( IID_PPV_ARGS( &gCaptureClient ) );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get capture client" );
      return FALSE;
   }

   hCaptureThread = CreateThread( NULL, 0, captureThread, NULL, 0, NULL );
   if ( hCaptureThread == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to create the capture thread" );
      return FALSE;
   }

   return TRUE;
}


BOOL cleanupAudioDevice() {

   if ( hCaptureThread != NULL ) {
      CloseHandle( hCaptureThread );
      hCaptureThread = NULL;
   }

   SafeRelease( &gCaptureClient );

   if ( gAudioSamplesReadyEvent != NULL ) {
      CloseHandle( gAudioSamplesReadyEvent );
      gAudioSamplesReadyEvent = NULL;
   }

   if ( glpAudioClient != NULL ) {
      glpAudioClient->Reset();
      glpAudioClient = NULL;
   }

   SafeRelease( &glpAudioClient );

   PropVariantClear( &DeviceFriendlyName );
   PropVariantClear( &DeviceDescription );
   PropVariantClear( &DeviceInterfaceFriendlyName );

   SafeRelease( &glpPropertyStore );

   if ( glpwstrDeviceId != NULL ) {
      CoTaskMemFree( glpwstrDeviceId );
      glpwstrDeviceId = NULL;
   }

   SafeRelease( &gDevice );

   return TRUE;
}
