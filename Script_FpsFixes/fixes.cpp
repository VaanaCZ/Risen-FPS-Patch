// ------------------------------------------------------------------------
// Risen - FPS fixes
// 
// Copyright (c) 2022 VaanaCZ
// ------------------------------------------------------------------------

#define ZSPY

#include <windows.h>
#ifdef ZSPY
#include <string>
#endif

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
	float x, y, z = 0.0f;
};

class eCTimer
{
public:

	float				GetFrameTimeInSeconds()										{ CALL(0x30039FC2); }
	static eCTimer*		GetInstance()												{ CALL(0x30039478); }

};

class eCDynamicEntity;

class gCSession
{
public:
	
	eCDynamicEntity*	GetPlayer()													{ CALL(0x2001012C); }
	static gCSession*	GetInstance()												{ CALL(0x20041DC6); }

};


enum BS_State
{
	BS_Standing = 1,
	BS_Sneaking = 2,
	BS_Walking = 3,
	BS_Running = 4,
	BS_Sprinting = 5,
	BS_Jumping = 6,
	BS_Jumping_climbing = 7,
	BS_Swimming = 8,
	BS_Diving = 9,
	//x              = 10; // drowned???
	BS_Wading = 11, // brodit se
	//x              = 12; // movemement
	BS_Levitating = 13,
	BS_Sliding = 14,
	BS_Falling = 15,
	BS_Dead = 16, // ragdoll
	BS_State_Max = 17
};

#define AAA(x) if (state == x) return #x

std::string GetStateName(BS_State state)
{
	if (state == 0)
	{
		return "BS_Standing WTF";
	}

	AAA(BS_Standing);
	AAA(BS_Sneaking);
	AAA(BS_Walking);
	AAA(BS_Running);
	AAA(BS_Sprinting);
	AAA(BS_Jumping);
	AAA(BS_Jumping_climbing);
	AAA(BS_Swimming);
	AAA(BS_Diving);
	AAA(BS_Wading);
	AAA(BS_Levitating);
	AAA(BS_Sliding);
	AAA(BS_Falling);
	AAA(BS_Dead);
}


//#include <intrin.h>
//#pragma intrinsic(_ReturnAddress)


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

	eCDynamicEntity*	GetGeometryEntity()											{ CALL(0x30048E7D); }

	void SetMovementMode2(int a0) { CALL(0x20191E40); }

	__declspec(safebuffers) void SetMovementMode_Hook(int a0)
	{
		/*
		gCSession* session = gCSession::GetInstance();
		eCDynamicEntity* player = session->GetPlayer();





		if (player == GetGeometryEntity())
		{
			void* rtn = _ReturnAddress();

			char buff[1024];
			sprintf_s(buff, "%x", (unsigned int)rtn);

			SendMessageToSpy(std::to_string(a0) + " " + GetStateName((BS_State)a0) + " 0x" + buff);


		}*/
		
		SetMovementMode2(a0);
	}

	void SetGoalPosition2(bCVector const& a0, bool a1, bool a2) { CALL(0x20187ED0); }

	__declspec(safebuffers) void SetGoalPosition_Hook(bCVector const& a0, bool a1, bool a2)
	{
		/*char buff[1024];
		sprintf_s(buff, "GoalPosition: %f %f %f", a0.x, a0.y, a0.z);

		SendMessageToSpy(buff);

		SetGoalPosition2(a0, a1, a2);*/
	}



};


#define FRAMETIME_30 0.0333333333f

float __stdcall fabsf_Hook(float val)
{
	eCTimer* timer = eCTimer::GetInstance();
	float frameTime = timer->GetFrameTimeInSeconds();
	
	float multiplier = FRAMETIME_30 / frameTime;

	//SendMessageToSpy("multiplier : " + std::to_string(multiplier));

	return fabsf(val) * multiplier;




}


unsigned long hookAddr = (unsigned long)&fabsf_Hook;


extern "C" __declspec(dllexport) void* __stdcall ScriptInit()
{
#ifdef ZSPY
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

	// ------------------------------------------------------------------------
	// Airwalk patch
	// ------------------------------------------------------------------------


	//auto hookAddr = &gCCharacterMovement_PS::SetMovementMode_Hook;
	//call hookCall = { 0xE9, (*(unsigned long*)&hookAddr) - 0x2004C3E8 - 5 };
	//WriteProcessMemory(GetCurrentProcess(), (void*)0x2004C3E8, &hookCall, 5, NULL);


	//auto hookAddr = &gCCharacterMovement_PS::SetGoalPosition_Hook;
	//call hookCall = { 0xE9, (*(unsigned long*)&hookAddr) - 0x20049D5F - 5 };
	//WriteProcessMemory(GetCurrentProcess(), (void*)0x20049D5F, &hookCall, 5, NULL);



	callPtr hookCall = { 0xFF, 0x15, ((unsigned long)&hookAddr) };
	WriteProcessMemory(GetCurrentProcess(), (void*)0x20194BC4, &hookCall, 6, NULL);



	return nullptr;
}
