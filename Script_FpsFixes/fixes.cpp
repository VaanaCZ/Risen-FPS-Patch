// ------------------------------------------------------------------------
// Risen - FPS fixes
// 
// Copyright (c) 2022 VaanaCZ
// ------------------------------------------------------------------------

#include <windows.h>
#ifdef _DEBUG
#include <string>
#endif

// ------------------------------------------------------------------------
// Debugging stuff
// ------------------------------------------------------------------------

#ifdef _DEBUG
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
	unsigned char opcode;  // FF
	unsigned char reg;     // 15
	unsigned long address; // 005E413C
};

struct call
{
	unsigned char opcode;  // E8
	unsigned long address; // 005E413C
};
#pragma pack(pop)

#define MASK 0xFF

void MaskPatch(DWORD address, char* bytes, size_t length, char mask)
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
	float x, y, z;
};

class gCCharacterMovement_PS
{
public:

	void		SetCurrentPosition(bCVector const& a0, bool a1, bool a2)	{ CALL(0x2004BA79); }
	bCVector	GetCurrentPosition()										{ CALL(0x200557F9); }

	__declspec(safebuffers) void AddToCurrentVelocity_Hook(bCVector const& a0, bool a1)
	{
		bCVector position = GetCurrentPosition();

		position.x += a0.x;
		position.y += a0.y;
		position.z += a0.z;

		SetCurrentPosition(position, true, true);
	}
};

extern "C" __declspec(dllexport) void* __stdcall ScriptInit()
{
#ifdef _DEBUG
	spyHandle = FindWindowA(0, "[zSpy]");
	SendMessageToSpy("FPS Fixes loaded");
#endif

	// ------------------------------------------------------------------------
	// ProcessClimbStandard patch
	// ------------------------------------------------------------------------

	// Remove divide call
	char standardPatch[] =
	{
		0x8B, 0xC1,	// mov eax, ecx
		0x59,		// pop ecx
		0x59,		// pop ecx
		0x90,		// nop
		0x90		// nop
	};

	MaskPatch(0x2018EF8B, standardPatch, sizeof(standardPatch), MASK);

	// AddToCurrentVelocity hook
	#define STANDARD_HOOK 0x2018EF94

	auto standardHookAddr = &gCCharacterMovement_PS::AddToCurrentVelocity_Hook;
	call standardHookCall = { 0xE8, (*(unsigned long*)&standardHookAddr) - STANDARD_HOOK - 5 };
	WriteProcessMemory(GetCurrentProcess(), (void*)STANDARD_HOOK, &standardHookCall, 5, NULL);

	// ------------------------------------------------------------------------
	// ProcessClimbHigh patch
	// ------------------------------------------------------------------------

	// Remove divide call
	char highPatch[] =
	{
		0x8B, 0xC1,	// mov eax, ecx
		0x59,		// pop ecx
		0x59,		// pop ecx
		0x90,		// nop
		0x90		// nop
	};

	MaskPatch(0x20193EBF, highPatch, sizeof(highPatch), MASK);

	// AddToCurrentVelocity hook
	#define HIGH_HOOK 0x20193EC8

	auto highHookAddr = &gCCharacterMovement_PS::AddToCurrentVelocity_Hook;
	call highHookCall = { 0xE8, (*(unsigned long*)&highHookAddr) - HIGH_HOOK - 5 };
	WriteProcessMemory(GetCurrentProcess(), (void*)HIGH_HOOK, &highHookCall, 5, NULL);

	return nullptr;
}
