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
#include <assert.h>  // For assert


IMMDevice*      gDevice = NULL;
LPWSTR          glpwstrDeviceId = NULL;
DWORD           glpdwState = 0;
IPropertyStore* glpPropertyStore = NULL;
PROPVARIANT     DeviceInterfaceFriendlyName;  // Container for the friendly name of the audio adapter for the device
PROPVARIANT     DeviceDescription;            // Container for the device's description
PROPVARIANT     DeviceFriendlyName;           // Container for friendly name of the device



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
   HRESULT hr;

   hr = DeviceCollection->Item( DeviceIndex, &device );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get device" );
      return NULL;
   }

   hr = device->OpenPropertyStore( STGM_READ, &glpPropertyStore );
   SafeRelease( &device );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to open device property store" );
      return NULL;
   }

   PROPVARIANT friendlyName;
   PropVariantInit( &friendlyName );
   hr = glpPropertyStore->GetValue( PKEY_Device_FriendlyName, &friendlyName );
   SafeRelease( &glpPropertyStore );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to retrieve friendly name for the device" );
      return NULL;
   }

   wchar_t deviceName[ 128 ];
   
   hr = StringCbPrintf( deviceName, sizeof( deviceName ), L"%s (%s)", friendlyName.vt != VT_LPWSTR ? L"Unknown" : friendlyName.pwszVal, glpwstrDeviceId );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to format a friendly name for the device" );
      return NULL;
   }
   
   PropVariantClear( &friendlyName );
   CoTaskMemFree( glpwstrDeviceId );
   
   wchar_t* returnValue = _wcsdup( deviceName );
   if ( returnValue == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to allocate a buffer" );
      return NULL;
   }
   return returnValue;
}


BOOL initAudioDevice( HWND hWnd ) {
   /// Get IMMDeviceEnumerator from COM (CoCreateInstance)

   IMMDeviceEnumerator* deviceEnumerator = NULL;

   HRESULT hr = CoCreateInstance( __uuidof( MMDeviceEnumerator ), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &deviceEnumerator ) );
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
      return NULL;
   }

   /// Get the device's properties from the property store
   PropVariantInit( &DeviceInterfaceFriendlyName );
   PropVariantInit( &DeviceDescription );
   PropVariantInit( &DeviceFriendlyName );

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



   return TRUE;
}


BOOL cleanupAudioDevice() {

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
