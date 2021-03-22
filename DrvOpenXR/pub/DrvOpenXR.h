#pragma once
#include "../OpenOVR/Drivers/Backend.h"

namespace DrvOpenXR {
IBackend* CreateOpenXRBackend();

/**
 * Restart the session, usually because the system state changed to STOPPING.
 *
 * See BaseSystem::_OnPostFrame.
 */
void RestartSession();

#ifdef SUPPORT_VK
void VkGetPhysicalDevice(VkInstance instance, VkPhysicalDevice* out);
class TemporaryVk* GetTemporaryVk();
#endif
}; // namespace DrvOpenXR
