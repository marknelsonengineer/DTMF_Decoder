///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
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

#include "audio.h"


template <class T> void SafeRelease( T** ppT ) {
   if ( *ppT ) {
      ( *ppT )->Release();
      *ppT = NULL;
   }
}


//
//  Retrieves the device friendly name for a particular device in a device collection.  
//
//  The returned string was allocated using malloc() so it should be freed using free();
//
LPWSTR GetDeviceName( IMMDeviceCollection* DeviceCollection, UINT DeviceIndex ) {
   IMMDevice* device;
   LPWSTR deviceId;
   HRESULT hr;

   hr = DeviceCollection->Item( DeviceIndex, &device );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get device" );
      return NULL;
   }

   hr = device->GetId( &deviceId );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get device ID" );
      return NULL;
   }

   IPropertyStore* propertyStore;
   hr = device->OpenPropertyStore( STGM_READ, &propertyStore );
   SafeRelease( &device );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to open device property store" );
      return NULL;
   }

   PROPVARIANT friendlyName;
   PropVariantInit( &friendlyName );
   hr = propertyStore->GetValue( PKEY_Device_FriendlyName, &friendlyName );
   SafeRelease( &propertyStore );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to retrieve friendly name for the device" );
      return NULL;
   }

   wchar_t deviceName[ 128 ];
   
   hr = StringCbPrintf( deviceName, sizeof( deviceName ), L"%s (%s)", friendlyName.vt != VT_LPWSTR ? L"Unknown" : friendlyName.pwszVal, deviceId );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to format a friendly name for the device" );
      return NULL;
   }
   
   PropVariantClear( &friendlyName );
   CoTaskMemFree( deviceId );
   
   wchar_t* returnValue = _wcsdup( deviceName );
   if ( returnValue == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to allocate a buffer" );
      return NULL;
   }
   return returnValue;
}


BOOL initAudio( HWND hWnd ) {
   OutputDebugStringA( __FUNCTION__ ":  Starting" );

   HRESULT hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to initialize COM" );
      return FALSE;
   }

   IMMDeviceEnumerator* deviceEnumerator = NULL;

   hr = CoCreateInstance( __uuidof( MMDeviceEnumerator ), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &deviceEnumerator ) );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to instantiate device enumerator" );
      return FALSE;
   }

   IMMDeviceCollection* deviceCollection = NULL;

   hr = deviceEnumerator->EnumAudioEndpoints( eCapture, DEVICE_STATE_ACTIVE, &deviceCollection );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to retrieve device collection" );
      return FALSE;
   }

   UINT deviceCount = -1;

   hr = deviceCollection->GetCount( &deviceCount );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get device collection length" );
      return FALSE;
   }

   if ( deviceCount <= 0 ) {
      OutputDebugStringA( __FUNCTION__ ":  No audio devices found" );
      return FALSE;
   }

   for ( UINT i = 0 ; i < deviceCount ; i++ ) {
      LPWSTR deviceName;

      deviceName = GetDeviceName( deviceCollection, i );
      if ( deviceName == NULL ) {
         OutputDebugStringA( __FUNCTION__ ":  Could not get a device name" );
         return FALSE;
      }

      OutputDebugStringW ( deviceName );
      free( deviceName );
   }

   return TRUE;
}


/*
IMMDevice* device = NULL;
bool isDefaultDevice;
ERole role;

if ( !PickDevice( &device, &isDefaultDevice, &role ) )



bool PickDevice( IMMDevice** DeviceToUse, bool* IsDefaultDevice, ERole* DefaultDeviceRole ) {
   HRESULT hr;
   bool retValue = true;
   IMMDeviceEnumerator* deviceEnumerator = NULL;
   IMMDeviceCollection* deviceCollection = NULL;

   *IsDefaultDevice = false;   // Assume we're not using the default device.

   hr = CoCreateInstance( __uuidof( MMDeviceEnumerator ), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &deviceEnumerator ) );
   if ( FAILED( hr ) ) {
      printf( "Unable to instantiate device enumerator: %x\n", hr );
      retValue = false;
      goto Exit;
   }

   IMMDevice* device = NULL;

   //
   //  First off, if none of the console switches was specified, use the console device.
   //
   if ( !UseConsoleDevice && !UseCommunicationsDevice && !UseMultimediaDevice && OutputEndpoint == NULL ) {
       //
       //  The user didn't specify an output device, prompt the user for a device and use that.
       //
      hr = deviceEnumerator->EnumAudioEndpoints( eCapture, DEVICE_STATE_ACTIVE, &deviceCollection );
      if ( FAILED( hr ) ) {
         printf( "Unable to retrieve device collection: %x\n", hr );
         retValue = false;
         goto Exit;
      }
      */