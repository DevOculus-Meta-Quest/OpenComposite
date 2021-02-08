//
// Created by ZNix on 8/02/2021.
//

#pragma once

#include "TemporaryGraphics.h"

#include "../XrDriverPrivate.h"

class TemporaryVk : public TemporaryGraphics {
public:
	TemporaryVk();
	~TemporaryVk() override;

	const void* GetGraphicsBinding() const override { return &binding; }

private:
	XrGraphicsBindingVulkanKHR binding = {};

	VkInstance instance = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
};