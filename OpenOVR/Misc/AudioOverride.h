#pragma once
#include "stdafx.h"
#include <Windows.h>
#include <Audioclient.h>
#include <Mmsystem.h>
#include <atlbase.h>
#include <cassert>
#include <mmdeviceapi.h>
#include <string>
#include <functiondiscoverykeys_devpkey.h>
#include <iostream>

#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "winmm.lib")
#include <type_traits>
#include <stdint.h>

HRESULT find_output_device(std::wstring& output, string audioOutputName);
void set_app_default_audio_device(std::wstring device_id);
