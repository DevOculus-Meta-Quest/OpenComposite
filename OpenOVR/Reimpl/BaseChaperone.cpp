#include "stdafx.h"
#define BASE_IMPL
#include "BaseChaperone.h"

#include "libovr_wrapper.h"

using namespace std;
using namespace vr;

BaseChaperone::BaseChaperoneCalibrationState BaseChaperone::GetCalibrationState() {
	return ChaperoneCalibrationState_OK;
}
bool BaseChaperone::GetPlayAreaSize(float *pSizeX, float *pSizeZ) {
	ovrVector3f points[4];
	int pointsCount;

	ovrResult result = ovrSuccess;
	OOVR_FAILED_OVR_LOG(ovr_GetBoundaryGeometry(
		*ovr::session,
		ovrBoundary_PlayArea,
		points,
		&pointsCount
	));

	float xmin = points[0].x, xmax = points[0].x;
	float zmin = points[0].z, zmax = points[0].z;

	for (int i = 0; i < 4; i++) {
		float x = points[i].x, z = points[i].z;

		if (x < xmin)
			xmin = x;
		if (z < zmin)
			zmin = z;

		if (x > xmax)
			xmax = x;
		if (z > zmax)
			zmax = z;
	}

	*pSizeX = xmax - xmin;
	*pSizeZ = zmax - zmin;

	// TODO verify return value
	return true;
}
bool BaseChaperone::GetPlayAreaRect(HmdQuad_t *rect) {
	ovrVector3f points[4];
	int pointsCount;

	ovrResult result = ovrSuccess;
	OOVR_FAILED_OVR_LOG(ovr_GetBoundaryGeometry(
		*ovr::session,
		ovrBoundary_PlayArea,
		points,
		&pointsCount
	));

	// Lifted from ReVive
	// TODO add ReVive (MIT) licence to repo
	// TODO make it go counter-clockwise
	memcpy(points, rect->vCorners, 4 * sizeof(ovrVector3f));

	//string msg = to_string(status) + "," + to_string(pointsCount);
	//OOVR_LOG(msg.c_str());

	return true;
}
void BaseChaperone::ReloadInfo(void) {
	STUBBED();
}
void BaseChaperone::SetSceneColor(HmdColor_t color) {
	STUBBED();
}
void BaseChaperone::GetBoundsColor(HmdColor_t *pOutputColorArray, int nNumOutputColors, float flCollisionBoundsFadeDistance, HmdColor_t *pOutputCameraColor) {
	STUBBED();
}
bool BaseChaperone::AreBoundsVisible() {
	ovrBool out;

	// Ignore the result, which could tell us the boundraries are not set up.
	ovrResult result = ovrSuccess;
	OOVR_FAILED_OVR_LOG(ovr_GetBoundaryVisible(*ovr::session, &out));

	return out;
}
void BaseChaperone::ForceBoundsVisible(bool bForce) {
	ovr_RequestBoundaryVisible(*ovr::session, bForce);
}
