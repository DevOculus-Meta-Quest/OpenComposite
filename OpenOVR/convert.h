#pragma once

vr::HmdMatrix44_t O2S_m4(ovrMatrix4f const& input);

ovrEyeType S2O_eye(vr::EVREye eye); 
void O2S_v3f(ovrVector3f const& in, vr::HmdVector3_t& out);
void O2S_om34( OVR::Matrix4f const& in, vr::HmdMatrix34_t& out);

// Out should be an identity initially.
void S2O_om44(vr::HmdMatrix34_t const& in, OVR::Matrix4f& out);

ovrTextureFormat dxgiToOvrFormat(
  DXGI_FORMAT dxgi,
  vr::EColorSpace colourSpace);