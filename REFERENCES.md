# References

The Windows API is quite complex.  Here's all of the API functions I needed to
research to write DTMF_Decoder.


### Generic Win32
| API                    | Link                                                                                         |
|------------------------|----------------------------------------------------------------------------------------------|
| `CreateWindowW`        | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindoww         |
| `RegisterClassExW`     | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw      |
| `DefWindowProc`        | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-defwindowproca        |
| `GetMessage`           | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessage            |
| `DispatchMessage`      | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dispatchmessage       |
| `LoadAccelerators`     | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadacceleratorsa     |
| `LoadStringW`          | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadstringw           |
| `MAKEINTRESOURCE`      | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-makeintresourcea      |
| `SendMessage`          | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendmessage           |
| `UpdateWindow`         | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-updatewindow          |
| `ShowWindow`           | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow            |
| `InvalidateRect`       | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-invalidaterect        |
| `TranslateAccelerator` | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translateacceleratora |
| `TranslateMessage`     | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage      |
| `DialogBox`            | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dialogboxa            |
| `PostQuitMessage`      | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postquitmessage       |
| `CloseHandle`          | https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle       |
| `EndDialog`            | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enddialog             |
| `DestroyWindow`        | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-destroywindow         |


### COM
| API                 | Link                                                                                          |
|---------------------|-----------------------------------------------------------------------------------------------|
| `CoCreateInstance`  | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance |
| `CoInitializeEx`    | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex   |
| `CoTaskMemFree`     | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cotaskmemfree    |
| `PropVariantClear`  | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-propvariantclear |
| `CoUninitialize`    | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-couninitialize   |
| `IUnknown::Release` | https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release         |


### GDI & Direct2D
| API                                         | Link                                                                                                                                                                                                        |
|---------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `BeginPaint` (GDI)                          | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-beginpaint                                                                                                                           |
| `EndPaint` (GDI)                            | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-endpaint                                                                                                                             |
| `D2D1CreateFactory`                         | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-d2d1createfactory-r1                                                                                                                       |
| `DWriteCreateFactory`                       | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-dwritecreatefactory                                                                                                                    |
| `ID2D1RenderTarget::CreateHwndRenderTarget` | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1factory-createhwndrendertarget(constd2d1_render_target_properties__constd2d1_hwnd_render_target_properties__id2d1hwndrendertarget)    |
| `ID2D1RenderTarget::BeginDraw`              | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-begindraw                                                                                                                |
| `D2D1::RectF`                               | https://learn.microsoft.com/en-us/windows/win32/api/d2d1helper/nf-d2d1helper-rectf                                                                                                                          |
| `ID2D1RenderTarget::Clear`                  | https://learn.microsoft.com/en-us/windows/win32/direct2d/id2d1rendertarget-clear                                                                                                                            |
| `ID2D1RenderTarget::CreateSolidColorBrush`  | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-createsolidcolorbrush(constd2d1_color_f__id2d1solidcolorbrush)                                                           |
| `IDWriteFactory::CreateTextFormat`          | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritefactory-createtextformat                                                                                                        |
| `DrawRoundedRectangle`                      | https://learn.microsoft.com/en-us/dotnet/api/system.windows.media.drawingcontext.drawroundedrectangle?view=windowsdesktop-6.0                                                                               |
| `ID2D1RenderTarget::DrawText`               | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-drawtext(constwchar_uint32_idwritetextformat_constd2d1_rect_f__id2d1brush_d2d1_draw_text_options_dwrite_measuring_mode)  |
| `IDWriteTextFormat::SetParagraphAlignment`  | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritetextformat-setparagraphalignment                                                                                                |
| `IDWriteTextFormat::SetTextAlignment`       | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritetextformat-settextalignment                                                                                                     |
| `IDWriteTextFormat::SetWordWrapping`        | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritetextformat-setwordwrapping                                                                                                      |
| `ID2D1RenderTarget::EndDraw`                | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-enddraw                                                                                                                  |


### Audio
| API                                            | Link                                                                                                                           |
|------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------|
| `IMMDevice`                                    | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nn-mmdeviceapi-immdevice                                       |
| `IAudioClient`                                 | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nn-audioclient-iaudioclient                                    |
| `IAudioCaptureClient`                          | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nn-audioclient-iaudiocaptureclient                             |
| `WAVEFORMATEX`                                 | https://learn.microsoft.com/en-us/windows/win32/api/mmeapi/ns-mmeapi-waveformatex                                              |
| `WAVEFORMATEXTENSIBLE`                         | https://learn.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatextensible                                        |
| `IMMDeviceEnumerator::GetDefaultAudioEndpoint` | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdeviceenumerator-getdefaultaudioendpoint     |
| `IMMDevice::GetId`                             | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-getid                                 |
| `IMMDevice::GetState`                          | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-getstate                              |
| `IMMDevice::OpenPropertyStore`                 | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-openpropertystore                     |
| `PropVariantInit`                              | https://learn.microsoft.com/en-us/windows/win32/api/propidl/nf-propidl-propvariantinit                                         |
| `IPropertyStore::GetValue`                     | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/bb761473(v=vs.85)                                   |
| `IAudioClient::GetMixFormat`                   | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getmixformat                       |
| `IAudioClient::IsFormatSupported`              | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-isformatsupported                  |
| `IAudioClient::Initialize`                     | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-initialize                         |
| `IMMDevice::Activate`                          | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-activate                              |
| `IAudioClient::GetBufferSize`                  | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getbuffersize                      |
| `IAudioClient::GetDevicePeriod`                | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getdeviceperiod                    |
| `IAudioClient::GetService`                     | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getservice                         |
| `IAudioClient::SetEventHandle`                 | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-seteventhandle                     |
| `IAudioClient::Start`                          | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-start                              |
| `AvSetMmThreadCharacteristics`                 | https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avsetmmthreadcharacteristicsa                                 |
| `AvRevertMmThreadCharacteristics`              | https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avrevertmmthreadcharacteristics                               |
| `IAudioCaptureClient::GetBuffer`               | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudiocaptureclient-getbuffer                   |
| `IAudioCaptureClient::ReleaseBuffer`           | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudiocaptureclient-releasebuffer               |
| `IAudioClient::Stop`                           | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-stop                               |
| `IAudioClient::Reset`                          | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-reset                              |


### Debugging & Instrumentation
| API                  | Link                                                                                                                           |
|----------------------|--------------------------------------------------------------------------------------------------------------------------------|
| `_ASSERTE`           | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/assert-asserte-assert-expr-macros?view=msvc-170              |
| `wcslen`             | https://en.cppreference.com/w/c/string/wide/wcslen                                                                             |
| `va_arg`             | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170                    |
| `sprintf_s`          | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/sprintf-s-sprintf-s-l-swprintf-s-swprintf-s-l?view=msvc-170     |
| `vsprintf_s`         | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vsprintf-s-vsprintf-s-l-vswprintf-s-vswprintf-s-l?view=msvc-170 |
| `OutputDebugStringA` | https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-outputdebugstringa                                       |
| `_ASSERT_EXPR`       | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/assert-asserte-assert-expr-macros?view=msvc-170                 |
| `MessageBoxA`        | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxa                                                |
| `swprintf_s`         | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/sprintf-s-sprintf-s-l-swprintf-s-swprintf-s-l?view=msvc-170     |
| `vswprintf_s`        | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vsprintf-s-vsprintf-s-l-vswprintf-s-vswprintf-s-l?view=msvc-170 |
| `OutputDebugStringW` | https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-outputdebugstringw                                       |
| `MessageBoxW`        | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxw                                                |/// 


### CRT & Memory management
| API                | Link                                                                                         |
|--------------------|----------------------------------------------------------------------------------------------|
| `_malloc_dbg`      | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/malloc-dbg                 |
| `SecureZeroMemory` | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366877(v=vs.85) |
| `_free_dbg`        | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/free-dbg                   |
| `_CrtCheckMemory`  | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/crtcheckmemory             |


### Threads & synchronization
| API                      | Link                                                                                                    |
|--------------------------|---------------------------------------------------------------------------------------------------------|
| `CreateThread`           | https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createthread |
| `CreateEventA`           | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createeventa                   |
| `CreateEventEx`          | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createeventexa                 |
| `ThreadProc`             | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms686736(v=vs.85) |
| `SetEvent`               | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-setevent                       |
| `WaitForSingleObject`    | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject            |
| `WaitForMultipleObjects` | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects         |
| `ExitThread`             | https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitthread   |


### Pragmas
| API                            | Link                                                                           |
|--------------------------------|--------------------------------------------------------------------------------|
| `#pragma message`              | https://learn.microsoft.com/en-us/cpp/preprocessor/message?view=msvc-170       |
| `#pragma comment(lib, "avrt")` | https://learn.microsoft.com/en-us/cpp/preprocessor/comment-c-cpp?view=msvc-170 |


### Math
| API                 | Link                                                                                 |
|---------------------|--------------------------------------------------------------------------------------|
| `_USE_MATH_DEFINES` | https://learn.microsoft.com/en-us/cpp/c-runtime-library/math-constants?view=msvc-170 |
| `sinf`              | https://en.cppreference.com/w/c/numeric/math/sin                                     |
| `cosf`              | https://en.cppreference.com/w/c/numeric/math/cos                                     |
| `sqrtf`             | https://en.cppreference.com/w/c/numeric/math/sqrt                                    |
