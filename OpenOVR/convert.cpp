#include "stdafx.h"
#include "convert.h"

using namespace vr;

HmdMatrix44_t O2S_m4(ovrMatrix4f const& input)
{
  HmdMatrix44_t output;
  memcpy_s(output.m, sizeof(float[4][4]), input.M, sizeof(float[4][4]));

  return output;
}

ovrEyeType S2O_eye(EVREye eye)
{
  return eye == Eye_Left ? ovrEye_Left : ovrEye_Right;
}

void O2S_v3f(ovrVector3f const& in, vr::HmdVector3_t& out)
{
  out.v[0] = in.x;
  out.v[1] = in.y;
  out.v[2] = in.z;
}

void O2S_om34(OVR::Matrix4f const& in, HmdMatrix34_t& out)
{
  for (size_t y = 0; y < 3; y++) {
    for (size_t x = 0; x < 4; x++) {
      out.m[y][x] = in.M[y][x];
    }
  }
}

// out should be an Identity matrix.
void S2O_om44(HmdMatrix34_t const& in, OVR::Matrix4f& out)
{
  for (size_t y = 0; y < 3; y++) {
    for (size_t x = 0; x < 4; x++) {
      out.M[y][x] = in.m[y][x];
    }
  }
}

ovrTextureFormat dxgiToOvrFormat(
  DXGI_FORMAT dxgi,
  vr::EColorSpace colourSpace)
{
  // TODO is this really how it should work?
  // No idea why or how or what, but for now just force SRGB on as otherwise
  // it causes trouble.
  bool useSrgb = true; // colourSpace != vr::ColorSpace_Auto;

  switch (dxgi) {
#define MAPPING(name) \
			case DXGI_ ## name: \
				return OVR_ ## name;

#define C_MAPPING(name) \
			case DXGI_ ## name: \
			case DXGI_ ## name ## _SRGB: \
				return useSrgb ? OVR_ ## name ## _SRGB : OVR_ ## name;

    MAPPING(FORMAT_B5G6R5_UNORM);
    MAPPING(FORMAT_B5G5R5A1_UNORM);
    MAPPING(FORMAT_B4G4R4A4_UNORM);
    C_MAPPING(FORMAT_R8G8B8A8_UNORM);
    C_MAPPING(FORMAT_B8G8R8A8_UNORM);
    C_MAPPING(FORMAT_B8G8R8X8_UNORM);
    MAPPING(FORMAT_R16G16B16A16_FLOAT);
    MAPPING(FORMAT_R11G11B10_FLOAT);

  case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    return useSrgb ? OVR_FORMAT_R8G8B8A8_UNORM_SRGB : OVR_FORMAT_R8G8B8A8_UNORM;

#undef C_MAPPING
#undef MAPPING
  }

  return OVR_FORMAT_UNKNOWN;
}
