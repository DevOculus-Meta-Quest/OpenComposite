#include "stdafx.h"
#include "logging.h"
#include <fstream>
#include <sstream>

using namespace std;

static ofstream stream;

void oovr_log_raw(char const* file, long line, char const* func, char const* msg) {
	if (!stream.is_open()) {
		stream.open("openovr_log");
	}

  SYSTEMTIME st;
  ::GetLocalTime(&st);

  char buff[1024];
  sprintf_s(buff, "%.2d:%.2d:%.2d.%.3d TID:0x%04x    ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, ::GetCurrentThreadId());

	stream << buff << func << "  Line:" << line << "\t- " << (msg ? msg : "NULL") << endl;

#ifdef _DEBUG
	std::stringstream str;
	str << func << " line: " << line << "\t- " << (msg ? msg : "NULL") << endl;
	::OutputDebugStringA(str.str().c_str());
#endif
	//stream.flush();  // Uncomment if need to not lose fewer messages on crash, generally don't use - harms perf.

	// Do we need to close the stream or something? What about multiple threads?
}

void oovr_log_raw_format(char const* file, long line, char const* func, char const* msg, ...) {
	va_list args;
	va_start(args, msg);

	char buff[2048];
	vsnprintf(buff, sizeof(buff), msg, args);

	oovr_log_raw(file, line, func, buff);

	va_end(args);
}

void oovr_abort_raw(char const* file, long line, char const* func, char const* msg, char const* title) {
	if (title == nullptr) {
		title = "OpenComposite Error - info in log";
		OOVR_LOG("Abort!");
	}
	else {
		OOVR_LOG(title);
	}

	oovr_log_raw(file, line, func, msg);

	// Ensure everything gets written
	stream << flush;

	MessageBoxA(NULL, msg, title, MB_OK);
	exit(1);
}

const float math_pi = 3.14159265358979323846f;
