#pragma once

#include "InteractionProfile.h"

class ReverbG2InteractionProfile : public InteractionProfile {
public:
	ReverbG2InteractionProfile();

	const std::string& GetPath() const override;
	std::optional<const char*> GetOpenVRName() const override;
	glm::mat4 GetGripToSteamVRTransform(ITrackedDevice::HandType hand) const override;

protected:
	const LegacyBindings* GetLegacyBindings(const std::string& handPath) const override;
	glm::mat4 handTransform;
};
