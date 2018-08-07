#include "stdafx.h"
#include "CVRSettings002.h"

#define MCALL() OOVR_LOG("Method called!");

const char * CVRSettings_002::GetSettingsErrorNameFromEnum(EVRSettingsError eError) { MCALL();
	return base.GetSettingsErrorNameFromEnum(eError);
}
bool CVRSettings_002::Sync(bool bForce, EVRSettingsError * peError) { MCALL();
	return base.Sync(bForce, (int*)peError);
}
void CVRSettings_002::SetBool(const char * pchSection, const char * pchSettingsKey, bool bValue, EVRSettingsError * peError) { MCALL();
	return base.SetBool(pchSection, pchSettingsKey, bValue, (int*)peError);
}
void CVRSettings_002::SetInt32(const char * pchSection, const char * pchSettingsKey, int32_t nValue, EVRSettingsError * peError) { MCALL();
	return base.SetInt32(pchSection, pchSettingsKey, nValue, (int*)peError);
}
void CVRSettings_002::SetFloat(const char * pchSection, const char * pchSettingsKey, float flValue, EVRSettingsError * peError) { MCALL();
	return base.SetFloat(pchSection, pchSettingsKey, flValue, (int*)peError);
}
void CVRSettings_002::SetString(const char * pchSection, const char * pchSettingsKey, const char * pchValue, EVRSettingsError * peError) { MCALL();
	return base.SetString(pchSection, pchSettingsKey, pchValue, (int*)peError);
}
bool CVRSettings_002::GetBool(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) { MCALL();
	return base.GetBool(pchSection, pchSettingsKey, (int*)peError);
}
int32_t CVRSettings_002::GetInt32(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) { MCALL();
	return base.GetInt32(pchSection, pchSettingsKey, (int*)peError);
}
float CVRSettings_002::GetFloat(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) { MCALL();
	return base.GetFloat(pchSection, pchSettingsKey, (int*)peError);
}
void CVRSettings_002::GetString(const char * pchSection, const char * pchSettingsKey, VR_OUT_STRING() char * pchValue, uint32_t unValueLen, EVRSettingsError * peError) { MCALL();
	return base.GetString(pchSection, pchSettingsKey, pchValue, unValueLen, (int*)peError);
}
void CVRSettings_002::RemoveSection(const char * pchSection, EVRSettingsError * peError) { MCALL();
	return base.RemoveSection(pchSection, (int*)peError);
}
void CVRSettings_002::RemoveKeyInSection(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) { MCALL();
	return base.RemoveKeyInSection(pchSection, pchSettingsKey, (int*)peError);
}
