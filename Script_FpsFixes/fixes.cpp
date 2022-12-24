
#include <windows.h>
#include <string>
#include <vector>
#include <thread>

HWND spyHandle;

void SendMessageToSpy(std::string msg)
{
	COPYDATASTRUCT data;
	data.lpData = (char*)msg.c_str();
	data.cbData = msg.size() + 1;

	SendMessageA(spyHandle, WM_COPYDATA, (WPARAM)0, (LPARAM)&data);
}

class bCVector
{
public:
	float x, y, z;
};

#define CALL(addr)			\
 __asm { mov esp, ebp	}	\
 __asm { pop ebp		}	\
 __asm { mov eax, addr	}	\
 __asm { jmp eax		}

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



//unsigned long funcAddr = (unsigned long)&GetFrameTimeInSeconds;

unsigned int t1;
unsigned int t2;
unsigned int t3;

extern "C" __declspec(dllexport) void* __stdcall ScriptInit()
{


	// ------------------------------------------------------------------------
	// ProcessClimbStandard patch
	// ------------------------------------------------------------------------

	// Nop out frametime stuff
	char nops[] =
	{
		0x8B, 0xC1,	// mov eax, ecx
		0x59,		// pop ecx
		0x59,		// pop ecx
		0x90,		// nop
		0x90		// nop
	};

	MaskPatch(0x2018EF8B, nops, sizeof(nops), MASK);

	// AddToCurrentVelocity hook

#define HOOK 0x2018EF94

	auto hookAddr = &gCCharacterMovement_PS::AddToCurrentVelocity_Hook;
	call hookCall = { 0xE8, (*(unsigned long*)&hookAddr) - HOOK - 5 };
	WriteProcessMemory(GetCurrentProcess(), (void*)HOOK, &hookCall, 5, NULL);



	spyHandle = FindWindowA(0, "[zSpy]");
	

	SendMessageToSpy("test");



	return nullptr;
}
