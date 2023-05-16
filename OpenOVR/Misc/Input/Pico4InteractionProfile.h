#pragma once

#include "InteractionProfile.h"

class Pico4InteractionProfile : public InteractionProfile {
public:
	Pico4InteractionProfile();

	const std::string& GetPath() const override;
	std::optional<const char*> GetOpenVRName() const override;

protected:
	const LegacyBindings* GetLegacyBindings(const std::string& handPath) const override;
};
