// ------------------------------------------------------------------------
// Risen - FPS fixes
// 
// Copyright (c) 2022-2024 VaanaCZ and Auronen
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
#define THUNDER_HOOK		0x303A8263

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

extern "C" int _fltused = 0;

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

DWORD newBuffer[64]; // workaround for new operator
DWORD* newPtr = newBuffer;

BOOL HookCallPtr(DWORD addr, DWORD func)
{
	// DWORD* a = new DWORD;
	DWORD* a = newPtr++;
	*a = func;

	callPtr c = { 0xFF, 0x15, ((DWORD)a) };
	return WriteProcessMemory(GetCurrentProcess(), (void*)addr, &c, sizeof(callPtr), NULL);
}

BOOL HookCall(DWORD addr, DWORD func)
{
	call c = { 0xE8, func - addr - sizeof(call) };
	return WriteProcessMemory(GetCurrentProcess(), (void*)addr, &c, sizeof(call), NULL);
}

#define MASK 0xFF

BOOL MaskPatch(DWORD address, BYTE* bytes, size_t length, char mask)
{
	for (size_t i = 0; i < length; i++)
	{
		if (bytes[i] != mask)
		{
			return WriteProcessMemory(GetCurrentProcess(), (DWORD*)(address + i), &bytes[i], 1, NULL);
		}
	}
}

#define CALL(addr)			\
 __asm { mov esp, ebp	}	\
 __asm { pop ebp		}	\
 __asm { mov eax, addr	}	\
 __asm { jmp eax		}

#define CALLPTR(addr)		\
 __asm { mov esp, ebp	}	\
 __asm { pop ebp		}	\
 __asm { mov eax, addr	}	\
 __asm { mov eax, [eax]	}	\
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

__declspec(safebuffers) float __stdcall fabsf_Hook(float val)
{
	eCTimer* timer = eCTimer::GetInstance();
	float frameTime = timer->GetFrameTimeInSeconds();
	
	float multiplier = FRAMETIME_30 / frameTime;

	return fabsf(val) * multiplier;
}

class bCMersenneTwister
{
public:

	float				Rand(float a0)												{ CALLPTR(0x30AA6914); }

	__declspec(safebuffers) float Rand_Hook(float a0)
	{
		eCTimer* timer = eCTimer::GetInstance();
		float frameTime = timer->GetFrameTimeInSeconds();

		static float thunderCooldown = FRAMETIME_30;

		if (thunderCooldown <= 1.19209290E-07F)
		{
			thunderCooldown = FRAMETIME_30;

			// 0.9999999776 SOUND
			// 0.7499999832 VISUAL

			return Rand(a0);
		}
		else
		{
			thunderCooldown -= frameTime;
			return a0;
		}
	}
};

extern "C" __declspec(dllexport) void* __stdcall ScriptInit()
{
#ifdef ZSPY
	spyHandle = FindWindowA(0, "[zSpy]");
	SendMessageToSpy("FPS Fixes loaded");
#endif

	// ------------------------------------------------------------------------
	// *** Climbing patch ***
	// 
	// Whenever the player initiates a climb by standing next to a ledge and
	// pressing space, the game calculates the target position by gradually
	// interpolating between the player's position and the ledge. After doing
	// so, the game then "corrects" the player position to the target position
	// by setting the player's velocity and letting the physics system do the
	// rest of the work.
	//
	// Sadly, on high FPS this mechanism breaksand the game overcorrects by a
	// small amount each frame, which causes the player to jump upand down
	// super fast, eventually sending him either into the stratosphere or to
	// the depths of the earth.
	//
	// The "fix" bypasses the physics system entirely and instead sets the
	// target position of the player directly.
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
	// *** Airwalk patch ***
	// 
	// While in freefall, the game performs checks to see if the player has
	// landed on the ground so that it can return to a walking state and give
	// back control. This check works by comparing the player's current Y
	// coordinate to the one from the previous frame and seeing if the
	// difference between them is below a certain threshold (less than 5cm). 
	//
	// When the game is running at high FPS (100+), the difference in position
	// between individual frames becomes so small, that the game falsely
	// detects the player has landed.
	//
	// The patch fixes this by altering the check according to the current fps.
	// ------------------------------------------------------------------------

	auto hookFabsf = &fabsf_Hook;

	HookCallPtr(FALL_HOOK, *(DWORD*)&hookFabsf);

	// ------------------------------------------------------------------------
	// *** Thunderstorm patch ***
	// 
	// Every frame there is check which determines whether a new thunder effect
	// should be generated. If this check passes (1% chance for a sound effect
	// and 0.75% for a visual effect), then a new thunder effect is created.
	// 
	// The patch simply limits this check to 1/30 of an FPS. So that the
	// thunderstorm effects are generated at the correct frequency.
	// ------------------------------------------------------------------------

	auto hookRand = &bCMersenneTwister::Rand_Hook;

	HookCallPtr(THUNDER_HOOK, *(DWORD*)&hookRand);

	return nullptr;
}
