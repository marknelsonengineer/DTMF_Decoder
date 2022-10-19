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


IMMDevice* device = NULL;


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


BOOL initAudioDevice( HWND hWnd ) {
   IMMDeviceEnumerator* deviceEnumerator = NULL;

   HRESULT hr = CoCreateInstance( __uuidof( MMDeviceEnumerator ), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &deviceEnumerator ) );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to instantiate device enumerator" );
      return FALSE;
   }

   hr = deviceEnumerator->GetDefaultAudioEndpoint( eCapture, eMultimedia, &device );
   if ( FAILED( hr ) || device == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get default audio device" );
      return FALSE;
   }

   return TRUE;
}


BOOL cleanupAudioDevice() {
   SafeRelease( &device );

   return TRUE;
}
