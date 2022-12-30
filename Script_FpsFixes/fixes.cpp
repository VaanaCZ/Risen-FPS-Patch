// ------------------------------------------------------------------------
// Risen - FPS fixes
// 
// Copyright (c) 2022 VaanaCZ and Auronen
// ------------------------------------------------------------------------

#include <windows.h>
#include <cmath>
#ifdef ZSPY
#include <string>
#endif

// ------------------------------------------------------------------------
// Offsets - this is what you change when an update breaks the patch!!!
// ------------------------------------------------------------------------

#define CLIMB_STANDARD_NOP	0x2018EF8B
#define CLIMB_STANDARD_HOOK	0x2018EF94
#define CLIMB_HIGH_NOP		0x20193EBF
#define CLIMB_HIGH_HOOK		0x20193EC8
#define FALL_HOOK			0x20194BC4

// ------------------------------------------------------------------------
// Debugging stuff
// ------------------------------------------------------------------------

#ifdef ZSPY
HWND spyHandle;

void SendMessageToSpy(std::string msg)
{
	COPYDATASTRUCT data;
	data.lpData = (char*)msg.c_str();
	data.cbData = msg.size() + 1;

	SendMessageA(spyHandle, WM_COPYDATA, (WPARAM)0, (LPARAM)&data);
}
#endif

// ------------------------------------------------------------------------
// Patching utils
// ------------------------------------------------------------------------

#pragma pack(push, 1)
struct callPtr
{
	BYTE	opcode;  // FF
	BYTE	reg;     // 15
	DWORD	address;
};

struct call
{
	BYTE	opcode;  // E8
	DWORD	address;
};
#pragma pack(pop)

void HookCallPtr(DWORD addr, DWORD func)
{
	DWORD* a = new DWORD;
	*a = func;

	callPtr c = { 0xFF, 0x15, ((DWORD)a) };
	WriteProcessMemory(GetCurrentProcess(), (void*)addr, &c, sizeof(callPtr), NULL);
}

void HookCall(DWORD addr, DWORD func)
{
	call c = { 0xE8, func - addr - sizeof(call) };
	WriteProcessMemory(GetCurrentProcess(), (void*)addr, &c, sizeof(call), NULL);
}

#define MASK 0xFF

void MaskPatch(DWORD address, BYTE* bytes, size_t length, char mask)
{
	for (size_t i = 0; i < length; i++)
	{
		if (bytes[i] != mask)
		{
			WriteProcessMemory(GetCurrentProcess(), (DWORD*)(address + i), &bytes[i], 1, NULL);
		}
	}
}

#define CALL(addr)			\
 __asm { mov esp, ebp	}	\
 __asm { pop ebp		}	\
 __asm { mov eax, addr	}	\
 __asm { jmp eax		}

// ------------------------------------------------------------------------
// Patch implementation
// ------------------------------------------------------------------------

class bCVector
{
public:
	float x, y, z = 0.0f;
};

class eCTimer
{
public:

	float				GetFrameTimeInSeconds()										{ CALL(0x30039FC2); }
	static eCTimer*		GetInstance()												{ CALL(0x30039478); }

};

class gCCharacterMovement_PS
{
public:

	void				SetCurrentPosition(bCVector const& a0, bool a1, bool a2)	{ CALL(0x2004BA79); }
	bCVector			GetCurrentPosition()										{ CALL(0x200557F9); }
	void				AddToCurrentVelocity(bCVector const& a0, bool a1)			{ CALL(0x2000673F); }

	__declspec(safebuffers) void AddToCurrentVelocity_Hook(bCVector const& a0, bool a1)
	{
		// Y coordinate is handled separately
		bCVector position = GetCurrentPosition();
		position.y += a0.y;

		SetCurrentPosition(position, true, true);

		// X and Z coordinates are handled like normal
		eCTimer* timer = eCTimer::GetInstance();
		float frametime = timer->GetFrameTimeInSeconds();

		bCVector velocity;
		velocity.x = a0.x / frametime;
		velocity.z = a0.z / frametime;

		AddToCurrentVelocity(velocity, a1);
	}
};

#define FRAMETIME_30 0.0333333333f

float __stdcall fabsf_Hook(float val)
{
	eCTimer* timer = eCTimer::GetInstance();
	float frameTime = timer->GetFrameTimeInSeconds();
	
	float multiplier = FRAMETIME_30 / frameTime;

	return fabsf(val) * multiplier;
}

extern "C" __declspec(dllexport) void* __stdcall ScriptInit()
{
#ifdef ZSPY
	spyHandle = FindWindowA(0, "[zSpy]");
	SendMessageToSpy("FPS Fixes loaded");
#endif

	// ------------------------------------------------------------------------
	// Climbing patch
	// ------------------------------------------------------------------------

	auto hookAddToCurrentVelocity = &gCCharacterMovement_PS::AddToCurrentVelocity_Hook;

	/*
		ProcessClimbStandard
	*/

	// Remove divide call
	BYTE climbStandardNop[] =
	{
		0x8B, 0xC1,	// mov eax, ecx
		0x59,		// pop ecx
		0x59,		// pop ecx
		0x90,		// nop
		0x90		// nop
	};

	MaskPatch(CLIMB_STANDARD_NOP, climbStandardNop, sizeof(climbStandardNop), MASK);

	// AddToCurrentVelocity hook
	HookCall(CLIMB_STANDARD_HOOK, *(DWORD*)&hookAddToCurrentVelocity);

	/*
		ProcessClimbHigh patch
	*/

	// Remove divide call
	BYTE climbHighNop[] =
	{
		0x8B, 0xC1,	// mov eax, ecx
		0x59,		// pop ecx
		0x59,		// pop ecx
		0x90,		// nop
		0x90		// nop
	};

	MaskPatch(CLIMB_HIGH_NOP, climbHighNop, sizeof(climbHighNop), MASK);

	// AddToCurrentVelocity hook
	HookCall(CLIMB_HIGH_HOOK, *(DWORD*)&hookAddToCurrentVelocity);

	// ------------------------------------------------------------------------
	// Airwalk patch
	// ------------------------------------------------------------------------
	
	auto hookFabsf = &fabsf_Hook;

	HookCallPtr(FALL_HOOK, *(DWORD*)&hookFabsf);

	return nullptr;
}
