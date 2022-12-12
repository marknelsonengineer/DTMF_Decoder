References
==========

The Windows API is quite complex.  Here's all of the API functions I needed to
research to write DTMF_Decoder.

I've broken them down by (my own) categories and then alphabetically.


## Generic Win32 API
| API                        | Link                                                                                                         |
|----------------------------|--------------------------------------------------------------------------------------------------------------|
| `CloseHandle`              | https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle                       |
| `CreateWindowW`            | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindoww                         |
| `DefWindowProcW`           | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-defwindowprocw                        |
| `DestroyWindow`            | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-destroywindow                         |
| `DialogBoxW`               | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dialogboxw                            |
| `DispatchMessage`          | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dispatchmessage                       |
| `EnableMenuItem`           | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enablemenuitem                        |
| `EndDialog`                | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enddialog                             |
| `GetCurrentProcess`        | https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getcurrentprocess |
| `GetMenu`                  | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmenu                               |
| `GetMessage`               | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessage                            |
| `GetProcessImageFileNameW` | https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getprocessimagefilenamew                  | 
| `HIWORD`                   | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms632657(v=vs.85)                 |
| `InvalidateRect`           | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-invalidaterect                        |
| `LoadAcceleratorsW`        | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadacceleratorsw                     |
| `LoadCursorW`              | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadcursorw                           |
| `LoadIconW`                | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadiconw                             |
| `LoadStringW`              | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadstringw                           |
| `MAKEINTRESOURCEW`         | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-makeintresourcew                      |
| `PostMessageW`             | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postmessagew                          |
| `PostQuitMessage`          | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postquitmessage                       |
| `RegisterClassExW`         | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw                      |
| `SetDlgItemTextW`          | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setdlgitemtextw                       |
| `ShowWindow`               | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow                            |
| `TranslateAcceleratorW`    | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translateacceleratorw                 |
| `TranslateMessage`         | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage                      |
| `UpdateWindow`             | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-updatewindow                          |


## COM API
| API                 | Link                                                                                          |
|---------------------|-----------------------------------------------------------------------------------------------|
| `CoCreateInstance`  | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance |
| `CoInitializeEx`    | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex   |
| `CoTaskMemFree`     | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cotaskmemfree    |
| `CoUninitialize`    | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-couninitialize   |
| `IUnknown::Release` | https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release         |
| `PropVariantClear`  | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-propvariantclear |


## GDI & Direct2D API
| API                                         | Link                                                                                                                                                                                                        |
|---------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `BeginPaint` (GDI)                          | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-beginpaint                                                                                                                           |
| `D2D1CreateFactory`                         | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-d2d1createfactory-r1                                                                                                                       |
| `D2D1::RectF`                               | https://learn.microsoft.com/en-us/windows/win32/api/d2d1helper/nf-d2d1helper-rectf                                                                                                                          |
| `DWriteCreateFactory`                       | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-dwritecreatefactory                                                                                                                    |
| `DrawRoundedRectangle`                      | https://learn.microsoft.com/en-us/dotnet/api/system.windows.media.drawingcontext.drawroundedrectangle?view=windowsdesktop-6.0                                                                               |
| `EndPaint` (GDI)                            | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-endpaint                                                                                                                             |
| `ID2D1RenderTarget::BeginDraw`              | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-begindraw                                                                                                                |
| `ID2D1RenderTarget::Clear`                  | https://learn.microsoft.com/en-us/windows/win32/direct2d/id2d1rendertarget-clear                                                                                                                            |
| `ID2D1RenderTarget::CreateHwndRenderTarget` | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1factory-createhwndrendertarget(constd2d1_render_target_properties__constd2d1_hwnd_render_target_properties__id2d1hwndrendertarget)    |
| `ID2D1RenderTarget::CreateSolidColorBrush`  | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-createsolidcolorbrush(constd2d1_color_f__id2d1solidcolorbrush)                                                           |
| `ID2D1RenderTarget::DrawText`               | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-drawtext(constwchar_uint32_idwritetextformat_constd2d1_rect_f__id2d1brush_d2d1_draw_text_options_dwrite_measuring_mode)  |
| `ID2D1RenderTarget::EndDraw`                | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-enddraw                                                                                                                  |
| `IDWriteFactory::CreateTextFormat`          | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritefactory-createtextformat                                                                                                        |
| `IDWriteTextFormat::SetParagraphAlignment`  | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritetextformat-setparagraphalignment                                                                                                |
| `IDWriteTextFormat::SetTextAlignment`       | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritetextformat-settextalignment                                                                                                     |
| `IDWriteTextFormat::SetWordWrapping`        | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritetextformat-setwordwrapping                                                                                                      |
| `InvalidateRect`                            | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-invalidaterect                                                                                                                       |


## Audio API
| API                                            | Link                                                                                                                           |
|------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------|
| `AvRevertMmThreadCharacteristics`              | https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avrevertmmthreadcharacteristics                               |
| `AvSetMmThreadCharacteristics`                 | https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avsetmmthreadcharacteristicsa                                 |
| `IAudioCaptureClient`                          | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nn-audioclient-iaudiocaptureclient                             |
| `IAudioCaptureClient::GetBuffer`               | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudiocaptureclient-getbuffer                   |
| `IAudioCaptureClient::ReleaseBuffer`           | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudiocaptureclient-releasebuffer               |
| `IAudioClient`                                 | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nn-audioclient-iaudioclient                                    |
| `IAudioClient::GetBufferSize`                  | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getbuffersize                      |
| `IAudioClient::GetDevicePeriod`                | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getdeviceperiod                    |
| `IAudioClient::GetMixFormat`                   | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getmixformat                       |
| `IAudioClient::GetService`                     | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getservice                         |
| `IAudioClient::Initialize`                     | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-initialize                         |
| `IAudioClient::IsFormatSupported`              | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-isformatsupported                  |
| `IAudioClient::Reset`                          | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-reset                              |
| `IAudioClient::SetEventHandle`                 | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-seteventhandle                     |
| `IAudioClient::Start`                          | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-start                              |
| `IAudioClient::Stop`                           | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-stop                               |
| `IMMDevice`                                    | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nn-mmdeviceapi-immdevice                                       |
| `IMMDevice::Activate`                          | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-activate                              |
| `IMMDeviceEnumerator::GetDefaultAudioEndpoint` | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdeviceenumerator-getdefaultaudioendpoint     |
| `IMMDevice::GetId`                             | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-getid                                 |
| `IMMDevice::GetState`                          | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-getstate                              |
| `IMMDevice::OpenPropertyStore`                 | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-openpropertystore                     |
| `IPropertyStore`                               | https://learn.microsoft.com/en-us/windows/win32/api/propsys/nn-propsys-ipropertystore                                          |
| `IPropertyStore::GetValue`                     | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/bb761473(v=vs.85)                                   |
| `PropVariantInit`                              | https://learn.microsoft.com/en-us/windows/win32/api/propidl/nf-propidl-propvariantinit                                         |
| `WAVEFORMATEX`                                 | https://learn.microsoft.com/en-us/windows/win32/api/mmeapi/ns-mmeapi-waveformatex                                              |
| `WAVEFORMATEXTENSIBLE`                         | https://learn.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatextensible                                        |


## Debugging & Instrumentation API
| API                      | Link                                                                                                                              |
|--------------------------|-----------------------------------------------------------------------------------------------------------------------------------|
| `_ASSERTE`               | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/assert-asserte-assert-expr-macros?view=msvc-170                 |
| `_ASSERT_EXPR`           | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/assert-asserte-assert-expr-macros?view=msvc-170                 |
| `MessageBeep`            | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebeep                                                |
| `MessageBoxA`            | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxa                                                |
| `MessageBoxW`            | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxw                                                |
| `OutputDebugStringA`     | https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-outputdebugstringa                                       |
| `OutputDebugStringW`     | https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-outputdebugstringw                                       |
| `WerRegisterMemoryBlock` | https://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werregistermemoryblock |
| `WerReportAddDump`       | https://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werreportadddump       |
| `WerReportCloseHandle`   | https://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werreportclosehandle   |
| `WerReportCreate`        | https://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werreportcreate        |
| `WerReportSetParameter`  | https://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werreportsetparameter  |
| `WerReportSubmit`        | https://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werreportsubmit        |
| `sprintf_s`              | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/sprintf-s-sprintf-s-l-swprintf-s-swprintf-s-l?view=msvc-170     |
| `swprintf_s`             | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/sprintf-s-sprintf-s-l-swprintf-s-swprintf-s-l?view=msvc-170     |
| `va_arg`                 | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170                    |
| `vsprintf_s`             | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vsprintf-s-vsprintf-s-l-vswprintf-s-vswprintf-s-l?view=msvc-170 |
| `vswprintf_s`            | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vsprintf-s-vsprintf-s-l-vswprintf-s-vswprintf-s-l?view=msvc-170 |
| `wcslen`                 | https://en.cppreference.com/w/c/string/wide/wcslen                                                                                |


## CRT & Memory Management API
| API                | Link                                                                                         |
|--------------------|----------------------------------------------------------------------------------------------|
| `CopyMemory`       | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366535(v=vs.85) |
| `_CrtCheckMemory`  | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/crtcheckmemory             |
| `SecureZeroMemory` | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366877(v=vs.85) |
| `StringCbCopyExW`  | https://learn.microsoft.com/en-us/windows/win32/api/strsafe/nf-strsafe-stringcbcopyexw       |
| `StringCbPrintfW`  | https://learn.microsoft.com/en-us/windows/win32/api/strsafe/nf-strsafe-stringcbprintfw       |
| `StringCchCopyW`   | https://learn.microsoft.com/en-us/windows/win32/api/strsafe/nf-strsafe-stringcchcopyw        |
| `StringCchPrintfW` | https://learn.microsoft.com/en-us/windows/win32/api/strsafe/nf-strsafe-stringcchprintfw      |
| `ZeroMemory`       | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366920(v=vs.85) |
| `_free_dbg`        | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/free-dbg                   |
| `_malloc_dbg`      | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/malloc-dbg                 |
| `rand`             | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/rand?view=msvc-170         |


## Threads & Synchronization API
| API                      | Link                                                                                                    |
|--------------------------|---------------------------------------------------------------------------------------------------------|
| `CreateEventExW`         | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createeventexw                 |
| `CreateEventW`           | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createeventw                   |
| `CreateThread`           | https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createthread |
| `DeleteCriticalSection`     | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-deletecriticalsection |
| `EnterCriticalSection`      | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-entercriticalsection |
| `ExitThread`             | https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitthread   |
| `InitializeCriticalSection` | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializecriticalsection |
| `LeaveCriticalSection`      | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-leavecriticalsection |
| `ResetEvent`             | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-resetevent                     |
| `SetEvent`               | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-setevent                       |
| `ThreadProc`             | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms686736(v=vs.85)            |
| `WaitForMultipleObjects` | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects         |
| `WaitForSingleObject`    | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject            |


## Pragmas
| API                            | Link                                                                           |
|--------------------------------|--------------------------------------------------------------------------------|
| `#pragma comment(lib, "avrt")` | https://learn.microsoft.com/en-us/cpp/preprocessor/comment-c-cpp?view=msvc-170 |
| `#pragma message`              | https://learn.microsoft.com/en-us/cpp/preprocessor/message?view=msvc-170       |


## Math
| API                 | Link                                                                                 |
|---------------------|--------------------------------------------------------------------------------------|
| `_USE_MATH_DEFINES` | https://learn.microsoft.com/en-us/cpp/c-runtime-library/math-constants?view=msvc-170 |
| `cosf`              | https://en.cppreference.com/w/c/numeric/math/cos                                     |
| `sinf`              | https://en.cppreference.com/w/c/numeric/math/sin                                     |
| `sqrtf`             | https://en.cppreference.com/w/c/numeric/math/sqrt                                    |
