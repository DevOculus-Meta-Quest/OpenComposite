#include "stdafx.h"
#include "compositor.h"

#include "libovr_wrapper.h"

Compositor::~Compositor() {
	if (mOVRSwapChain) {
		ovr_DestroyTextureSwapChain(*ovr::session, mOVRSwapChain);
		mOVRSwapChain = NULL;
	}
}
