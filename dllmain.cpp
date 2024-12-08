// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "gameinput.h"
#include "aixlog.hpp"
#include "xinput.h"
#include "steam/isteaminput.h"

#define LOG_FUNCTION_CALL do {															\
	static bool emitted = false;														\
	if (!emitted)																		\
	{																					\
		LOG(AixLog::Severity::info) << "function invoked" << std::endl;					\
		emitted = true;																	\
	}																					\
} while (0)

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD ul_reason_for_call,
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

// internals from steam that we need to make our pSteamInput() accessor work
typedef HSteamUser (*SteamAPI_GetHSteamUserFn)();
typedef void* (*SteamInternal_FindOrCreateUserInterfaceFn)(HSteamUser, const char *);
typedef void* (*SteamInternal_ContextInitFn)(void **);
typedef void (*SteamInternal_Init_SteamInputFn)(ISteamInput **);
static SteamAPI_GetHSteamUserFn pSteamAPI_GetHSteamUser = nullptr;
static SteamInternal_FindOrCreateUserInterfaceFn pSteamInternal_FindOrCreateUserInterface = nullptr;
static SteamInternal_ContextInitFn pSteamInternal_ContextInit = nullptr;

// expanded macro from isteaminput.h
// because we're doing this at runtime, we have to use pSteamInput()
// instead of SteamInput(), or else we'll get a linker error
inline void __cdecl pSteamInternal_Init_SteamInput(ISteamInput **p)
{
	*p = (ISteamInput *)(pSteamInternal_FindOrCreateUserInterface(
		pSteamAPI_GetHSteamUser(),
		STEAMINPUT_INTERFACE_VERSION
	));
}
inline ISteamInput *pSteamInput()
{
	static void *s_CallbackCounterAndContext[3] = { (void *)&pSteamInternal_Init_SteamInput, 0, 0 };
	return *(ISteamInput **)pSteamInternal_ContextInit(s_CallbackCounterAndContext);
}

static bool hasSteamInput(bool tryLoad = true)
{
	// we will try to get the steam_api64.dll module for 2.5 minutes after the game
	// has launched, after that we will give up
	static const int FIND_STEAM_INPUT_TIMEOUT_MS = 60 * 1000 * 2.5;
	static int findSteamInputDeadline = -1;
	static bool findSteamInputFailed = false;

	// we have the internals, so you can access SteamInput
	if (pSteamAPI_GetHSteamUser && pSteamInternal_FindOrCreateUserInterface && pSteamInternal_ContextInit)
		return true;

	// either we're not going to try to load it, or we already failed to find it
	if (!tryLoad || findSteamInputFailed)
		return false;

	// setup deadline, or if we're past it, log and return false
	if (findSteamInputDeadline == -1)
	{
		LOG(AixLog::Severity::info) << "Started searching for SteamInput" << std::endl;
		findSteamInputDeadline = GetTickCount() + FIND_STEAM_INPUT_TIMEOUT_MS;
	}
	else if (GetTickCount() > findSteamInputDeadline)
	{
		LOG(AixLog::Severity::error) << "SteamInput not found after timeout" << std::endl;
		findSteamInputFailed = true;
		return false;
	}

	// try to get the steam_api64.dll module
	HMODULE hSteamAPI = GetModuleHandleW(L"steam_api64.dll");

	// if we can't get the module, we'll try again later
	if (!hSteamAPI) return false;

	// get addresses of the internal functions

	pSteamAPI_GetHSteamUser =
		(SteamAPI_GetHSteamUserFn)GetProcAddress(hSteamAPI, "SteamAPI_GetHSteamUser");

	pSteamInternal_FindOrCreateUserInterface =
		(SteamInternal_FindOrCreateUserInterfaceFn)GetProcAddress(hSteamAPI, "SteamInternal_FindOrCreateUserInterface");

	pSteamInternal_ContextInit =
		(SteamInternal_ContextInitFn)GetProcAddress(hSteamAPI, "SteamInternal_ContextInit");

	if (!pSteamAPI_GetHSteamUser || !pSteamInternal_FindOrCreateUserInterface || !pSteamInternal_ContextInit)
	{
		LOG(AixLog::Severity::error) << "SteamAPI internal functions not found" << std::endl;
		findSteamInputFailed = true;
		pSteamAPI_GetHSteamUser = nullptr;
		pSteamInternal_FindOrCreateUserInterface = nullptr;
		pSteamInternal_ContextInit = nullptr;
		return false;
	}

	LOG(AixLog::Severity::info) << "SteamAPI internal functions found" << std::endl;

	// TODO: unsure of steam input initialization behavior if something else already initialized it,
	// so out of safety we'll try doing a test call to see if it's usable
	try {
		if (pSteamInput()->Init(false))
			LOG(AixLog::Severity::info) << "SteamInput initialized by gameinput2xinput" << std::endl;
		else
			pSteamInput()->GetSessionInputConfigurationSettings();
	} catch (...) {
		LOG(AixLog::Severity::error) << "SteamInput initialization failed" << std::endl;
		findSteamInputFailed = true;
		return false;
	}

	// we did it!
	LOG(AixLog::Severity::info) << "SteamInput ready" << std::endl;

	return true;
}

class GameInputDevice : public IGameInputDevice {
private:
	GameInputDeviceState* _deviceState;

public:
	explicit GameInputDevice(GameInputDeviceState* deviceState) : _deviceState(deviceState)
	{
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

		// TODO: it may or may not matter, but we may want to change the controller vid/pid
		// to match an xbox one controller in case games don't send impulse triggers due to
		// the device family being 360
		// for stalker 2, it does not matter

		dev_info.vendorId = 0x45e;
		dev_info.productId = 0x28e;

		dev_info.deviceFamily = GameInputFamilyXbox360;
		dev_info.usage.id = 5;
		dev_info.usage.page = 1;

		dev_info.interfaceNumber = 0;

		dev_info.supportedInput = GameInputKindControllerAxis | GameInputKindControllerButton | GameInputKindGamepad | GameInputKindUiNavigation;
		dev_info.supportedRumbleMotors = GameInputRumbleLowFrequency | GameInputRumbleHighFrequency| GameInputRumbleLeftTrigger | GameInputRumbleRightTrigger;

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

		auto xinputSlot = _deviceState->xinputSlot;

		if (xinputSlot != -1)
		{
			UINT16 low = params != nullptr ? static_cast <UINT16> (params->lowFrequency * 65536.0f) : 0ui16;
			UINT16 high = params != nullptr ? static_cast <UINT16> (params->highFrequency * 65536.0f) : 0ui16;
			UINT16 left = params != nullptr ? static_cast <UINT16> (params->leftTrigger * 65536.0f) : 0ui16;
			UINT16 right = params != nullptr ? static_cast <UINT16> (params->rightTrigger * 65536.0f): 0ui16;

			// take the max of high, left, right, since this may be a controller that does not
			// have trigger vibration and there is no guarantee that the low and high frequency
			// motors are on the left and right side respectively, for e.g. the Dualsense which
			// just has two identical motors on each side and uses a rumble emulation that fires
			// both motors at the same time
			UINT16 highMixed = max(max(left, right), high);

			if (hasSteamInput())
			{
				InputHandle_t handle = pSteamInput()->GetControllerForGamepadIndex(_deviceState->xinputSlot);

				// this controller may not be associated with steam input
				if (handle)
				{
					// send left and right trigger only if it's an impulse trigger controller
					if (pSteamInput()->GetInputTypeForHandle(handle) == k_ESteamInputType_XBoxOneController)
						pSteamInput()->TriggerVibrationExtended(handle, low, high, left, right);
					else
						pSteamInput()->TriggerVibrationExtended(handle, low, highMixed, 0, 0);

					// return now so we don't fall back to xinput
					return;
				}
			}

			// fall back to xinput

			XINPUT_VIBRATION vibration = {
				static_cast<WORD>(min(65535U, low)),
				static_cast<WORD>(min(65535U, highMixed))
			};

			auto result = XInputSetState(xinputSlot, &vibration);

			if (result != ERROR_SUCCESS)
			{
				LOG(AixLog::Severity::error) << "xinput set state error: " << result << " slot: " << xinputSlot << std::endl;
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
	XINPUT_STATE _lastXinputState = {};
	GameInputDeviceState* _deviceState;
	bool _logAllXinputErrorsOnce = true;

public:
	explicit GameInputReading(GameInputDeviceState* deviceState) : _deviceState(deviceState)
	{
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

		if (xinputSlot == -1)
		{
			for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
			{
				int result = XInputGetState(i, &xinputState);

				if (result == ERROR_SUCCESS)
				{
					LOG(AixLog::Severity::info) << "using xinput slot: " << i << std::endl;
					xinputSuccess = true;
					_deviceState->xinputSlot = i;
					break;
				}
				else
				{
					if (_logAllXinputErrorsOnce)
					{
						LOG(AixLog::Severity::error) << "xinput error: " << result << " slot: " << i << std::endl;
					}
				}
			}

			_logAllXinputErrorsOnce = false;
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
				LOG(AixLog::Severity::error) << "xinput error: " << result << " slot: " << xinputSlot << std::endl;
				_logAllXinputErrorsOnce = true;
			}
		}

		if (xinputSuccess)
		{
			if (std::exchange(_lastXinputState.dwPacketNumber, xinputState.dwPacketNumber) < xinputState.dwPacketNumber)
			{
				QueryPerformanceCounter(&_timestamp);
			}

			ConvertXInputToGameInput(xinputState, state);
		}
		else
		{
			_timestamp = {};
			_deviceState->xinputSlot = -1;
			_lastXinputState.dwPacketNumber = 0;
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
	UINT64 _lastGamepadReading = 0;

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

		// if we have steam input, we should shut it down
		if (hasSteamInput(false))
		{
			LOG(AixLog::Severity::info) << "shutting down steam input" << std::endl;
			pSteamInput()->Shutdown();
		}

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
				if (std::exchange(_lastGamepadReading, _reading.GetTimestamp()) < _reading.GetTimestamp())
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
			LOG(AixLog::Severity::info) << "calling device callback with our fake device" << std::endl;
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
