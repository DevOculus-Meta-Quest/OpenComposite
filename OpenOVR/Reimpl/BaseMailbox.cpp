#include "stdafx.h"

#define BASE_IMPL

#include "BaseMailbox.h"

typedef BaseMailbox::MboxErr MboxErr;

MboxErr BaseMailbox::RegisterMailbox(const char* name, OOVR_mbox_handle* handle)
{
	*handle = 123;

	// Ignore it for now
	OOVR_LOGF("Pretending to register mailbox '%s'", name);

	return VR_MBox_None;
}

MboxErr BaseMailbox::undoc2(OOVR_mbox_handle a)
{
	//STUBBED();
	OOVR_LOGF("Pretending to deregister mailbox");
	return VR_MBox_None;
}

MboxErr BaseMailbox::undoc3(OOVR_mbox_handle a, const char* maybeTargetMboxName, const char* message)
{
	// Seems to be a send function?

	OOVR_LOGF("Pretending to send mailbox message '%s' by '%s'", maybeTargetMboxName, message);

	return VR_MBox_None;
}

MboxErr BaseMailbox::ReadMessage(OOVR_mbox_handle mboxHandle, char* outBuf, uint32_t outBufLen, uint32_t* outMsgLen)
{
	// Read message function
	// If we have a message available (IDK what the mailbox is even supposed to do), try to read a message.
	// If the message fits within the output buffer, copy it in and return 0.
	// Otherwise if it doesn't fit, return 2.
	// In both cases, set msgLen
	// If we don't have a message, return an unknown error code other than 0 and 2 (HL:A doesn't care).

	// OOVR_LOGF("MBox poll");

	static bool hitMsg = false;
	if(!hitMsg) {
		const std::string msg = R"(
{
	"type": "ready",
}
)";
		*outMsgLen = msg.size();

		if(outBufLen < msg.size()) {
			return VR_MBox_BufferTooShort;
		}

		memcpy(outBuf, msg.c_str(), msg.size());

		hitMsg = true;

		OOVR_LOGF("Sending fake message %s", msg.c_str())
		return VR_MBox_None;
	}

	// Fow now, just say we don't have a message
	return (MboxErr)12345;
}
