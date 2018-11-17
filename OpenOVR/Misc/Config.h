#pragma once

class Config
{
public:
  Config();
  ~Config();

  float SupersampleRatio() const { return supersampleRatio; }
  inline bool ThreePartSubmit() const { return threePartSubmit; }
  inline bool UseViewportStencil() const { return useViewportStencil; }
  inline bool DX10Mode() const { return dx10Mode; }
  inline bool EnableLayers() const { return enableLayers; }
  inline bool EnableCubemap() const { return enableCubemap; }
  inline std::string PostInitCmd() const { return postInitCmd; }

private:
  static int ini_handler(
    void* user, const char* section,
    const char* name, const char* value,
    int lineno);

  float supersampleRatio = 1.0f;

  // rF2 defaults.
  bool dx10Mode = false;
  bool enableLayers = false;
  bool enableCubemap = true;
  bool threePartSubmit = true;
  bool useViewportStencil = true;
  std::string postInitCmd;
};

extern Config oovr_global_configuration;
