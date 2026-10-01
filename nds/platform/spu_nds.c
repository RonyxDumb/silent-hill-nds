#include <string.h>
#include <psx/libspu.h>

static unsigned int s_keyMask;
static unsigned int s_reverbMask;
static unsigned int s_transferAddress;
static int s_reverbEnabled;

void SpuInit(void) {}
int SpuInitMalloc(int count, char* top) { (void)count; (void)top; return 0; }
void SpuSetVoiceAttr(SpuVoiceAttr* attribute) { (void)attribute; }
void SpuGetVoiceAttr(SpuVoiceAttr* attribute) { if (attribute != NULL) memset(attribute, 0, sizeof(*attribute)); }
void SpuSetKey(int onOff, unsigned int voiceMask) { if (onOff != 0) s_keyMask |= voiceMask; else s_keyMask &= ~voiceMask; }
void SpuSetKeyOnWithAttr(SpuVoiceAttr* attribute) { if (attribute != NULL) SpuSetKey(SPU_ON, attribute->voice); }
int SpuGetKeyStatus(unsigned int voiceMask) { return (s_keyMask & voiceMask) != 0 ? SPU_ON : SPU_OFF; }
int SpuSetReverb(int onOff) { int previous = s_reverbEnabled; s_reverbEnabled = onOff; return previous; }
int SpuSetReverbModeParam(SpuReverbAttr* attribute) { (void)attribute; return SPU_SUCCESS; }
unsigned int SpuSetReverbVoice(int onOff, unsigned int voiceMask) { if (onOff != 0) s_reverbMask |= voiceMask; else s_reverbMask &= ~voiceMask; return s_reverbMask; }
unsigned int SpuGetReverbVoice(void) { return s_reverbMask; }
int SpuReserveReverbWorkArea(int onOff) { return onOff; }
int SpuClearReverbWorkArea(int mode) { (void)mode; return SPU_SUCCESS; }
unsigned int SpuWrite(unsigned char* address, unsigned int size) { (void)address; return size; }
int SpuSetTransferMode(int mode) { return mode; }
unsigned int SpuSetTransferStartAddr(unsigned int address) { unsigned int previous = s_transferAddress; s_transferAddress = address; return previous; }
int SpuIsTransferCompleted(int flag) { (void)flag; return 1; }
void SpuSetCommonAttr(SpuCommonAttr* attribute) { (void)attribute; }
