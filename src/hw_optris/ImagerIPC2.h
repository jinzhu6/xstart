#pragma once
// Imager IPC.h
// Definitions for Imager Interprocess Communication

#ifdef __cplusplus 
extern "C"
{
#endif

#ifdef IMAGERIPC_EXPORTS
#define IMAGERIPC_API __declspec(dllexport)
#else
#define IMAGERIPC_API __declspec(dllimport)
#endif

#ifndef WINAPI
#define WINAPI __stdcall 
#endif

#ifndef HRESULT
#define HRESULT long
#endif

enum TFlagState {fsFlagOpen, fsFlagClose, fsFlagOpening, fsFlagClosing, fsError};
struct FrameMetadata
{
	unsigned short Size;	// size of this structure
	unsigned int Counter;	// frame counter
	unsigned int CounterHW;	// frame counter hardware
	long long Timestamp;	// time stamp in UNITS (10000000 per second)
	long long TimestampMedia;
	TFlagState FlagState;
	float TempChip;
	float TempFlag;
	float TempBox;
	WORD PIFin;
};


// type definitions for function pointer
typedef HRESULT (WINAPI *fpOnServerStopped)(int reason);
typedef HRESULT (WINAPI *fpOnFrameInit)(int, int, int);
typedef HRESULT (WINAPI *fpOnNewFrame)(char*, int);
typedef HRESULT (WINAPI *fpOnNewFrameEx)(void*, FrameMetadata*);
typedef HRESULT (WINAPI *fpOnInitCompleted)(void );
typedef HRESULT (WINAPI *fpOnConfigChanged)(long reserved);
typedef HRESULT (WINAPI *fpOnFileCommandReady)(wchar_t *Path);


IMAGERIPC_API HRESULT WINAPI SetImagerIPCCount(WORD count);
IMAGERIPC_API HRESULT WINAPI InitImagerIPC(WORD index);
IMAGERIPC_API HRESULT WINAPI InitNamedImagerIPC(WORD index, wchar_t *InstanceName);
IMAGERIPC_API HRESULT WINAPI RunImagerIPC(WORD index);
IMAGERIPC_API HRESULT WINAPI ReleaseImagerIPC(WORD index);
IMAGERIPC_API HRESULT WINAPI AcknowledgeFrame(WORD index);
IMAGERIPC_API HRESULT WINAPI SetLogFile(wchar_t *LogFilename, int LogLevel, bool Append);

//--------------------------------------------------------------------------------------
// Callback procedures:
IMAGERIPC_API HRESULT WINAPI SetCallback_OnServerStopped(WORD index, fpOnServerStopped OnServerStopped);
IMAGERIPC_API HRESULT WINAPI SetCallback_OnFrameInit(WORD index, fpOnFrameInit OnFrameInit);
IMAGERIPC_API HRESULT WINAPI SetCallback_OnNewFrame(WORD index, fpOnNewFrame OnNewFrame);
IMAGERIPC_API HRESULT WINAPI SetCallback_OnNewFrameEx(WORD index, fpOnNewFrameEx OnNewFrameEx);
IMAGERIPC_API HRESULT WINAPI SetCallback_OnVisibleFrameInit(WORD index, fpOnFrameInit OnVisibleFrameInit);
IMAGERIPC_API HRESULT WINAPI SetCallback_OnNewVisibleFrame(WORD index, fpOnNewFrame OnNewVisibleFrame);
IMAGERIPC_API HRESULT WINAPI SetCallback_OnNewVisibleFrameEx(WORD index, fpOnNewFrameEx OnNewVisibleFrameEx);
IMAGERIPC_API HRESULT WINAPI SetCallback_OnInitCompleted(WORD index, fpOnInitCompleted OnInitCompleted);
IMAGERIPC_API HRESULT WINAPI SetCallback_OnConfigChanged(WORD index, fpOnConfigChanged OnConfigChanged);
IMAGERIPC_API HRESULT WINAPI SetCallback_OnFileCommandReady(WORD index, fpOnFileCommandReady OnFileCommandReady);

//--------------------------------------------------------------------------------------
// Get & Set procedures: 
IMAGERIPC_API __int64 WINAPI GetVersionApplication(WORD index);
IMAGERIPC_API __int64 WINAPI GetVersionHID_DLL(WORD index);
IMAGERIPC_API __int64 WINAPI GetVersionCD_DLL(WORD index);
IMAGERIPC_API __int64 WINAPI GetVersionIPC_DLL(WORD index);

IMAGERIPC_API float WINAPI GetTempChip(WORD index);
IMAGERIPC_API float WINAPI GetTempFlag(WORD index);
IMAGERIPC_API float WINAPI GetTempProc(WORD index); 
IMAGERIPC_API float WINAPI GetTempBox(WORD index);
IMAGERIPC_API float WINAPI GetTempHousing(WORD index);
IMAGERIPC_API float WINAPI GetTempTarget(WORD index);
IMAGERIPC_API float WINAPI GetHumidity(WORD index);
IMAGERIPC_API USHORT WINAPI GetTempRangeCount(WORD index);
IMAGERIPC_API USHORT WINAPI GetOpticsCount(WORD index);
IMAGERIPC_API USHORT WINAPI GetMeasureAreaCount(WORD index);
IMAGERIPC_API float WINAPI GetTempMinRange(WORD index, ULONG Index);
IMAGERIPC_API float WINAPI GetTempMaxRange(WORD index, ULONG Index);
IMAGERIPC_API USHORT WINAPI GetOpticsFOV(WORD index, ULONG Index);
IMAGERIPC_API float WINAPI GetTempMeasureArea(WORD index, ULONG Index);
IMAGERIPC_API USHORT WINAPI GetInitCounter(WORD index);

IMAGERIPC_API bool WINAPI GetFlag(WORD index);
IMAGERIPC_API bool WINAPI SetFlag(WORD index, bool Value);
IMAGERIPC_API USHORT WINAPI GetOpticsIndex(WORD index);
IMAGERIPC_API USHORT WINAPI SetOpticsIndex(WORD index, USHORT Value);
IMAGERIPC_API USHORT WINAPI GetTempRangeIndex(WORD index);
IMAGERIPC_API USHORT WINAPI SetTempRangeIndex(WORD index, USHORT Value);
IMAGERIPC_API USHORT WINAPI GetTempRangeDecimal(WORD index, bool Effective);
IMAGERIPC_API bool   WINAPI GetMainWindowEmbedded(WORD index);
IMAGERIPC_API bool   WINAPI SetMainWindowEmbedded(WORD index, bool Value);
IMAGERIPC_API USHORT WINAPI GetMainWindowLocX(WORD index);
IMAGERIPC_API USHORT WINAPI SetMainWindowLocX(WORD index, USHORT Value);
IMAGERIPC_API USHORT WINAPI GetMainWindowLocY(WORD index);
IMAGERIPC_API USHORT WINAPI SetMainWindowLocY(WORD index, USHORT Value);
IMAGERIPC_API USHORT WINAPI GetMainWindowWidth(WORD index);
IMAGERIPC_API USHORT WINAPI SetMainWindowWidth(WORD index, USHORT Value);
IMAGERIPC_API USHORT WINAPI GetMainWindowHeight(WORD index);
IMAGERIPC_API USHORT WINAPI SetMainWindowHeight(WORD index, USHORT Value);
IMAGERIPC_API float	 WINAPI GetFixedEmissivity(WORD index);
IMAGERIPC_API float	 WINAPI SetFixedEmissivity(WORD index, float Value);
IMAGERIPC_API float	 WINAPI GetFixedTransmissivity(WORD index);
IMAGERIPC_API float	 WINAPI SetFixedTransmissivity(WORD index, float Value);
IMAGERIPC_API float	 WINAPI GetFixedTempAmbient(WORD index);
IMAGERIPC_API float	 WINAPI SetFixedTempAmbient(WORD index, float Value);

IMAGERIPC_API UCHAR  WINAPI GetHardware_Model(WORD index);
IMAGERIPC_API UCHAR  WINAPI GetHardware_Spec(WORD index);
IMAGERIPC_API ULONG  WINAPI GetSerialNumber(WORD index);
IMAGERIPC_API ULONG  WINAPI GetSerialNumberULIS(WORD index);
IMAGERIPC_API USHORT WINAPI GetFirmware_MSP(WORD index);
IMAGERIPC_API USHORT WINAPI GetFirmware_Cypress(WORD index);
IMAGERIPC_API USHORT WINAPI GetPID(WORD index);
IMAGERIPC_API USHORT WINAPI GetVID(WORD index);

//--------------------------------------------------------------------------------------
// Control commands:
IMAGERIPC_API void WINAPI ResetFlag(WORD index);
IMAGERIPC_API bool WINAPI RenewFlag(WORD index);
IMAGERIPC_API void WINAPI CloseApplication(WORD index);
IMAGERIPC_API void WINAPI ReinitDevice(WORD index);
IMAGERIPC_API void WINAPI FileSnapshot(WORD index);
IMAGERIPC_API void WINAPI FileRecord(WORD index);
IMAGERIPC_API void WINAPI FileStop(WORD index); 
IMAGERIPC_API void WINAPI FilePlay(WORD index);
IMAGERIPC_API void WINAPI FilePause(WORD index); 
IMAGERIPC_API USHORT WINAPI FileOpen(WORD index, wchar_t * FileName);
IMAGERIPC_API USHORT WINAPI LoadLayout(WORD index, wchar_t * LayoutName); 

#ifdef __cplusplus 
}
#endif