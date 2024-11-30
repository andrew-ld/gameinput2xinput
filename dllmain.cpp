// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "gameinput.h"
#include "aixlog.hpp"

#define LOG_FUNCTION_CALL LOG(INFO) << "function invoked" << std::endl;

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		auto sink_cout = std::make_shared<AixLog::SinkCout>(AixLog::Severity::trace);
		auto sink_file = std::make_shared<AixLog::SinkFile>(AixLog::Severity::trace, "gameinput.log");
		AixLog::Log::init({ sink_cout, sink_file });
		LOG_FUNCTION_CALL;
	}

	return TRUE;
}

class GameInputDevice : public IGameInputDevice {
public:
	HRESULT QueryInterface(const IID& riid, void** ppvObj) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	ULONG AddRef() noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	ULONG Release() noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	// https://github.com/SpecialKO/SpecialK/blob/3efff581925870736e07247aea4c370f889787a4/src/input/game_input.cpp#L876
	// GNU General Public License 3
	const GameInputDeviceInfo* GetDeviceInfo() noexcept override
	{
		LOG_FUNCTION_CALL;

		static GameInputDeviceInfo dev_info = {};

		dev_info.infoSize = sizeof(GameInputDeviceInfo);

		dev_info.controllerAxisCount = 6;
		dev_info.controllerButtonCount = 13;

		dev_info.deviceId = { 1 };
		dev_info.deviceRootId = { 1 };

		dev_info.capabilities = GameInputDeviceCapabilityPowerOff | GameInputDeviceCapabilityWireless;

		dev_info.vendorId = 0x45e;
		dev_info.productId = 0x28e;

		dev_info.deviceFamily = GameInputFamilyXbox360;
		dev_info.usage.id = 5;
		dev_info.usage.page = 1;

		dev_info.interfaceNumber = 0;

		dev_info.supportedInput = GameInputKindControllerAxis | GameInputKindControllerButton | GameInputKindGamepad | GameInputKindUiNavigation;
		dev_info.supportedRumbleMotors = GameInputRumbleLowFrequency | GameInputRumbleHighFrequency;

		return &dev_info;
	}

	GameInputDeviceStatus GetDeviceStatus() noexcept override
	{
		LOG_FUNCTION_CALL;
		return GameInputDeviceStatus();
	}

	void GetBatteryState(GameInputBatteryState* state) noexcept override
	{
		LOG_FUNCTION_CALL;
	}

	HRESULT CreateForceFeedbackEffect(uint32_t motorIndex, const GameInputForceFeedbackParams* params, IGameInputForceFeedbackEffect** effect) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	bool IsForceFeedbackMotorPoweredOn(uint32_t motorIndex) noexcept override
	{
		LOG_FUNCTION_CALL;
		return false;
	}

	void SetForceFeedbackMotorGain(uint32_t motorIndex, float masterGain) noexcept override
	{
		LOG_FUNCTION_CALL;
	}

	HRESULT SetHapticMotorState(uint32_t motorIndex, const GameInputHapticFeedbackParams* params) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	void SetRumbleState(const GameInputRumbleParams* params) noexcept override
	{
		LOG_FUNCTION_CALL;
	}

	void SetInputSynchronizationState(bool enabled) noexcept override
	{
		LOG_FUNCTION_CALL;
	}

	void SendInputSynchronizationHint() noexcept override
	{
		LOG_FUNCTION_CALL;
	}

	void PowerOff() noexcept override
	{
		LOG_FUNCTION_CALL;
	}

	HRESULT CreateRawDeviceReport(uint32_t reportId, GameInputRawDeviceReportKind reportKind, IGameInputRawDeviceReport** report) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT GetRawDeviceFeature(uint32_t reportId, IGameInputRawDeviceReport** report) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT SetRawDeviceFeature(IGameInputRawDeviceReport* report) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT SendRawDeviceOutput(IGameInputRawDeviceReport* report) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT SendRawDeviceOutputWithResponse(IGameInputRawDeviceReport* requestReport, IGameInputRawDeviceReport** responseReport) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT ExecuteRawDeviceIoControl(uint32_t controlCode, size_t inputBufferSize, const void* inputBuffer, size_t outputBufferSize, void* outputBuffer, size_t* outputSize) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	bool AcquireExclusiveRawDeviceAccess(uint64_t timeoutInMicroseconds) noexcept override
	{
		LOG_FUNCTION_CALL;
		return false;
	}

	void ReleaseExclusiveRawDeviceAccess() noexcept override
	{
		LOG_FUNCTION_CALL;
	}
};

class GameInputReading : public IGameInputReading {
public:
	HRESULT QueryInterface(const IID& riid, void** ppvObj) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	ULONG AddRef() noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	ULONG Release() noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	GameInputKind GetInputKind() noexcept override
	{
		LOG_FUNCTION_CALL;
		return GameInputKindGamepad;
	}
	uint64_t GetSequenceNumber(GameInputKind inputKind) noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	uint64_t GetTimestamp() noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	void GetDevice(IGameInputDevice** device) noexcept override
	{
		LOG_FUNCTION_CALL;
	}

	bool GetRawReport(IGameInputRawDeviceReport** report) noexcept override
	{
		LOG_FUNCTION_CALL;
		return false;
	}

	uint32_t GetControllerAxisCount() noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	uint32_t GetControllerAxisState(uint32_t stateArrayCount, float* stateArray) noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	uint32_t GetControllerButtonCount() noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	uint32_t GetControllerButtonState(uint32_t stateArrayCount, bool* stateArray) noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	uint32_t GetControllerSwitchCount() noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	uint32_t GetControllerSwitchState(uint32_t stateArrayCount, GameInputSwitchPosition* stateArray) noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	uint32_t GetKeyCount() noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	uint32_t GetKeyState(uint32_t stateArrayCount, GameInputKeyState* stateArray) noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	bool GetMouseState(GameInputMouseState* state) noexcept override
	{
		LOG_FUNCTION_CALL;
		return false;
	}

	uint32_t GetTouchCount() noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	uint32_t GetTouchState(uint32_t stateArrayCount, GameInputTouchState* stateArray) noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	bool GetMotionState(GameInputMotionState* state) noexcept override
	{
		LOG_FUNCTION_CALL;
		return false;
	}

	bool GetArcadeStickState(GameInputArcadeStickState* state) noexcept override
	{
		LOG_FUNCTION_CALL;
		return false;
	}

	bool GetFlightStickState(GameInputFlightStickState* state) noexcept override
	{
		LOG_FUNCTION_CALL;
		return false;
	}

	bool GetGamepadState(GameInputGamepadState* state) noexcept override
	{
		LOG_FUNCTION_CALL;

		

		return false;
	}

	bool GetRacingWheelState(GameInputRacingWheelState* state) noexcept override
	{
		LOG_FUNCTION_CALL;
		return false;
	}

	bool GetUiNavigationState(GameInputUiNavigationState* state) noexcept override
	{
		LOG_FUNCTION_CALL;
		return false;
	}
};

class GameInput : public IGameInput {
private:
	GameInputDevice _device{};
	GameInputReading _reading{};

public:
	HRESULT QueryInterface(const IID& riid, void** ppvObj) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	ULONG AddRef() noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	ULONG Release() noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	uint64_t GetCurrentTimestamp() noexcept override
	{
		LOG_FUNCTION_CALL;
		return 0;
	}

	HRESULT GetCurrentReading(GameInputKind inputKind, IGameInputDevice* device, IGameInputReading** reading) noexcept override
	{
		LOG_FUNCTION_CALL;

		if (device == &_device) {
			*reading = &_reading;
			return S_OK;
		}

		return E_NOTIMPL;
	}

	HRESULT GetNextReading(IGameInputReading* referenceReading, GameInputKind inputKind, IGameInputDevice* device, IGameInputReading** reading) noexcept override
	{
		LOG_FUNCTION_CALL;

		if (device == &_device) {
			return GAMEINPUT_E_READING_NOT_FOUND;
		}

		return E_NOTIMPL;
	}

	HRESULT GetPreviousReading(IGameInputReading* referenceReading, GameInputKind inputKind, IGameInputDevice* device, IGameInputReading** reading) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT GetTemporalReading(uint64_t timestamp, IGameInputDevice* device, IGameInputReading** reading) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT RegisterReadingCallback(IGameInputDevice* device, GameInputKind inputKind, float analogThreshold, void* context, GameInputReadingCallback callbackFunc, GameInputCallbackToken* callbackToken) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT RegisterDeviceCallback(IGameInputDevice* device, GameInputKind inputKind, GameInputDeviceStatus statusFilter, GameInputEnumerationKind enumerationKind, void* context, GameInputDeviceCallback callbackFunc, GameInputCallbackToken* callbackToken) noexcept override
	{
		LOG_FUNCTION_CALL;

		if (inputKind == GameInputKindGamepad && (statusFilter & GameInputDeviceConnected) != 0) {
			LARGE_INTEGER timestamp;
			QueryPerformanceCounter(&timestamp);
			callbackFunc(callbackToken != nullptr ? *callbackToken : 0, context, &_device, timestamp.QuadPart, GameInputDeviceConnected, GameInputDeviceConnected);
			return S_OK;
		}

		return E_NOTIMPL;
	}

	HRESULT RegisterGuideButtonCallback(IGameInputDevice* device, void* context, GameInputGuideButtonCallback callbackFunc, GameInputCallbackToken* callbackToken) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT RegisterKeyboardLayoutCallback(IGameInputDevice* device, void* context, GameInputKeyboardLayoutCallback callbackFunc, GameInputCallbackToken* callbackToken) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	void StopCallback(GameInputCallbackToken callbackToken) noexcept override
	{
		LOG_FUNCTION_CALL;
	}

	bool UnregisterCallback(GameInputCallbackToken callbackToken, uint64_t timeoutInMicroseconds) noexcept override
	{
		LOG_FUNCTION_CALL;
		return false;
	}

	HRESULT CreateDispatcher(IGameInputDispatcher** dispatcher) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT CreateAggregateDevice(GameInputKind inputKind, IGameInputDevice** device) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT FindDeviceFromId(const APP_LOCAL_DEVICE_ID* value, IGameInputDevice** device) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT FindDeviceFromObject(IUnknown* value, IGameInputDevice** device) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT FindDeviceFromPlatformHandle(HANDLE value, IGameInputDevice** device) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT FindDeviceFromPlatformString(LPCWSTR value, IGameInputDevice** device) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	HRESULT EnableOemDeviceSupport(uint16_t vendorId, uint16_t productId, uint8_t interfaceNumber, uint8_t collectionNumber) noexcept override
	{
		LOG_FUNCTION_CALL;
		return E_NOTIMPL;
	}

	void SetFocusPolicy(GameInputFocusPolicy policy) noexcept override
	{
		LOG_FUNCTION_CALL;
	}
};

extern "C" __declspec(dllexport) HRESULT __stdcall GameInputCreate(
	_COM_Outptr_ IGameInput** gameInput) {
	LOG_FUNCTION_CALL;

	static GameInput gameInputSingleton{};

	*gameInput = &gameInputSingleton;

	return S_OK;
}
