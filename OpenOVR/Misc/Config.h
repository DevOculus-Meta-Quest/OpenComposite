#pragma once

class Config {
public:
	Config();
	~Config();

	bool EnableAudio() const { return enableAudio; }
	bool RenderCustomHands() const { return renderCustomHands; }
	vr::HmdColor_t HandColour() const { return handColour; }
	float SupersampleRatio() const { return supersampleRatio; }
	float Haptics() const { return haptics; }
	float AdmitUnknownProps() const { return admitUnknownProps; }
	inline bool ThreePartSubmit() const { return threePartSubmit; }
	inline bool UseViewportStencil() const { return useViewportStencil; }
	inline bool ForceConnectedTouch() const { return forceConnectedTouch; }
	inline bool LogGetTrackedProperty() const { return logGetTrackedProperty; }
	inline bool DX10Mode() const { return dx10Mode; }
	inline bool EnableLayers() const { return enableLayers; }
	inline bool EnableCubemap() const { return enableCubemap; }
	inline std::string PostInitCmd() const { return postInitCmd; }

private:
	static int ini_handler(
		void* user, const char* section,
		const char* name, const char* value,
		int lineno);

	// TODO: Remove/ifdefout.
	bool enableAudio = false;
	bool renderCustomHands = false;
	vr::HmdColor_t handColour = vr::HmdColor_t{ 0.3f, 0.3f, 0.3f, 1 };
	float supersampleRatio = 1.0f;
	bool haptics = false;
	bool admitUnknownProps = false;
	bool forceConnectedTouch = false;
	bool logGetTrackedProperty = false;

	// rF2 defaults.
	bool dx10Mode = false;
	bool enableLayers = false;
	bool enableCubemap = true;
	bool threePartSubmit = true;
	bool useViewportStencil = true;
	std::string postInitCmd;
};

extern Config oovr_global_configuration;
