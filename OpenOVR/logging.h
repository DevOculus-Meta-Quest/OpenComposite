#pragma once

void oovr_log_raw(const char *file, long line, const char *func, const char *msg);
void oovr_log_raw_format(const char *file, long line, const char *func, const char *msg, ...);
#define OOVR_LOG(msg) oovr_log_raw(__FILE__, __LINE__, __FUNCTION__, msg);
#define OOVR_LOGF(...) oovr_log_raw_format(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);

void oovr_abort_raw(const char *file, long line, const char *func, const char *msg, const char *title = nullptr);
#define OOVR_ABORT(msg) { oovr_abort_raw(__FILE__, __LINE__, __FUNCTION__, msg); }
#define OOVR_ABORT_T(msg, title) { oovr_abort_raw(__FILE__, __LINE__, __FUNCTION__, msg, title); }

// DirectX API validation helpers
#define OOVR_FAILED_DX_LOG(expression) hr = expression; if (FAILED(hr)) { OOVR_LOGF("DX Call failed with: 0x%08x", hr); }

// General validation helpers
#define OOVR_FALSE_LOG(expression) if (!(expression)) { OOVR_LOGF("Expression is false unexpectedly"); }
#define OOVR_FALSE_ABORT(expression) if (!(expression)) { OOVR_ABORT("Expression is false unexpectedly"); }

// OVR API validation helpers
#define OOVR_FAILED_OVR_LOG(expression) result = expression; \
if (!OVR_SUCCESS(result)) { \
	ovrErrorInfo e = {}; \
	ovr_GetLastErrorInfo(&e); \
	OOVR_LOGF("OVR Call failed.  Error code: %d  Descr: %s", e.Result, e.ErrorString); \
}

#define OOVR_FAILED_OVR_ABORT(expression) result = expression; \
if (!OVR_SUCCESS(result)) { \
	string err = string(__FILE__) + " line: " + to_string(__LINE__) + " error: " + to_string(result); \
	OOVR_FAILED_OVR_LOG(expression); \
	OOVR_ABORT(err.c_str()); \
}

// Yay for there not being a PI constant in the standard
// (technically it has absolutely nothing to do with logging but this is a convenient place to put it)
const extern float math_pi;