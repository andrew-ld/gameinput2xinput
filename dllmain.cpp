// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "gameinput.h"
#include "aixlog.hpp"
#include "xinput.h"
#include <algorithm>

#define LOG_FUNCTION_CALL do {															\
	static bool emitted = false;														\
	if (!emitted)																		\
	{																					\
		LOG(AixLog::Severity::info) << "function invoked" << std::endl;					\
		emitted = true;																	\
	}																					\
} while (0)

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		auto sink_cout = std::make_shared<AixLog::SinkCout>(AixLog::Severity::trace);
		auto sink_file = std::make_shared<AixLog::SinkFile>(AixLog::Severity::trace, "gameinput.log");
		AixLog::Log::init({ sink_cout, sink_file });
		LOG_FUNCTION_CALL;
	}

	return TRUE;
}

// https://github.com/SpecialKO/SpecialK/blob/245e0c06b4cc2785972a35adbe0b4d1552a83b5b/src/input/game_input.cpp#L1859
// GNU General Public License 3
static void ConvertXInputToGameInput(const XINPUT_STATE& xinputState, GameInputGamepadState* state)
{
	const XINPUT_GAMEPAD& xgamepad = xinputState.Gamepad;

	state->buttons = GameInputGamepadNone;

	state->leftThumbstickX =
		static_cast <float> (static_cast <double> (xgamepad.sThumbLX) / 32767.0);
	state->leftThumbstickY =
		static_cast <float> (static_cast <double> (xgamepad.sThumbLY) / 32767.0);

	state->rightThumbstickX =
		static_cast <float> (static_cast <double> (xgamepad.sThumbRX) / 32767.0);
	state->rightThumbstickY =
		static_cast <float> (static_cast <double> (xgamepad.sThumbRY) / 32767.0);

	state->leftTrigger =
		static_cast <float> (static_cast <double> (xgamepad.bLeftTrigger) / 255.0);
	state->rightTrigger =
		static_cast <float> (static_cast <double> (xgamepad.bRightTrigger) / 255.0);

	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_A) != 0 ? GameInputGamepadA : GameInputGamepadNone;
	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_B) != 0 ? GameInputGamepadB : GameInputGamepadNone;
	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_X) != 0 ? GameInputGamepadX : GameInputGamepadNone;
	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_Y) != 0 ? GameInputGamepadY : GameInputGamepadNone;

	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_START) != 0 ? GameInputGamepadMenu : GameInputGamepadNone;
	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0 ? GameInputGamepadView : GameInputGamepadNone;

	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0 ? GameInputGamepadDPadUp : GameInputGamepadNone;
	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0 ? GameInputGamepadDPadDown : GameInputGamepadNone;
	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0 ? GameInputGamepadDPadLeft : GameInputGamepadNone;
	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0 ? GameInputGamepadDPadRight : GameInputGamepadNone;

	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0 ? GameInputGamepadLeftShoulder : GameInputGamepadNone;
	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0 ? GameInputGamepadRightShoulder : GameInputGamepadNone;

	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0 ? GameInputGamepadLeftThumbstick : GameInputGamepadNone;
	state->buttons |= (xgamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0 ? GameInputGamepadRightThumbstick : GameInputGamepadNone;
}

class GameInputDeviceState {
public:
	int xinputSlot = -1;
};

class GameInputDevice : public IGameInputDevice {
private:
	GameInputDeviceState *_deviceState;

public:
	explicit GameInputDevice(GameInputDeviceState *deviceState) : _deviceState(deviceState) {
		LOG_FUNCTION_CALL;
	}

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

		XINPUT_VIBRATION vibration = {
			params != nullptr ? static_cast<WORD>(min(65535U, static_cast<UINT>(min(max(params->leftTrigger + params->lowFrequency,  0.0f), 1.0f) * 65536.0f))) : 0ui16,
			params != nullptr ? static_cast<WORD>(min(65535U, static_cast<UINT>(min(max(params->rightTrigger + params->highFrequency, 0.0f), 1.0f) * 65536.0f))) : 0ui16
		};

		auto xinputSlot = _deviceState->xinputSlot;

		if (xinputSlot != -1)
		{
			auto result = XInputSetState(xinputSlot, &vibration);

			if (result != ERROR_SUCCESS) {
				LOG(AixLog::Severity::error) << "xinput set state error: " << result << std::endl;
			}
		}
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
private:
	LARGE_INTEGER _timestamp = {};
	XINPUT_STATE _last_state = {};
	GameInputDeviceState *_deviceState;

public:
	explicit GameInputReading(GameInputDeviceState *deviceState) : _deviceState(deviceState) {
		LOG_FUNCTION_CALL;
	}

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
		return _timestamp.QuadPart;
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

		XINPUT_STATE xinputState;
		bool xinputSuccess = false;

		ZeroMemory(&xinputState, sizeof(XINPUT_STATE));
		ZeroMemory(state, sizeof(GameInputGamepadState));

		auto xinputSlot = _deviceState->xinputSlot;

		if (xinputSlot == -1) {
			for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
			{
				if (XInputGetState(i, &xinputState) == ERROR_SUCCESS)
				{
					xinputSuccess = true;
					_deviceState->xinputSlot = i;
					break;
				}
			}
		}
		else
		{
			int result = XInputGetState(xinputSlot, &xinputState);

			if (result == ERROR_SUCCESS)
			{
				xinputSuccess = true;
			}
			else
			{
				LOG(AixLog::Severity::info) << "xinput error: " << result << std::endl;
			}
		}

		if (xinputSuccess)
		{
			if (std::exchange(_last_state.dwPacketNumber, xinputState.dwPacketNumber) < xinputState.dwPacketNumber)
			{
				QueryPerformanceCounter(&_timestamp);
			}

			ConvertXInputToGameInput(xinputState, state);
		}
		else
		{
			_timestamp = {};
			_deviceState->xinputSlot = -1;
			_last_state.dwPacketNumber = 0;
		}

		return xinputSuccess;
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
	GameInputDeviceState _deviceState{};
	GameInputDevice _device{ &_deviceState };
	GameInputReading _reading{ &_deviceState };
	UINT64 _last_gampad_reading = 0;

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

		if (device == &_device)
		{
			*reading = &_reading;
			return S_OK;
		}

		return E_NOTIMPL;
	}

	HRESULT GetNextReading(IGameInputReading* referenceReading, GameInputKind inputKind, IGameInputDevice* device, IGameInputReading** reading) noexcept override
	{
		LOG_FUNCTION_CALL;

		if (device == &_device)
		{
			GameInputGamepadState gamepad_state = {};
			if (_reading.GetGamepadState(&gamepad_state))
			{
				if (std::exchange(_last_gampad_reading, _reading.GetTimestamp()) < _reading.GetTimestamp())
				{
					*reading = &_reading;
					return S_OK;
				}
			}

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

		if (inputKind == GameInputKindGamepad && (statusFilter & GameInputDeviceConnected) != 0)
		{
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

extern "C" __declspec(dllexport) HRESULT __stdcall GameInputCreate(_COM_Outptr_ IGameInput** gameInput)
{
	LOG_FUNCTION_CALL;

	static GameInput gameInputSingleton{};

	*gameInput = &gameInputSingleton;

	return S_OK;
}
