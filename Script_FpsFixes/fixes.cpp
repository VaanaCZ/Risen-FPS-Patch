
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


class bCVector;

//std::vector<float> frameTimes;
//std::vector<bCVector> vectors;

std::vector<float> heroYs;

std::vector<float> fvar6s;
std::vector<float> fvar5s;
std::vector<float> t2ecs;
std::vector<float> tc4s;
std::vector<float> results;


bCVector* heroPos;

int count = 0;

class bCVector
{
public:
	float x, y, z;

	typedef bCVector(bCVector::* OperatorDivide_t)(float);

	inline static bCVector::OperatorDivide_t originalOperatorDivideAddr;
	inline static bCVector::OperatorDivide_t newOperatorDivideAddr;

	__declspec(safebuffers) bCVector operator/(float scalar)
	{
		count++;

		results.push_back(y);

		//SendMessageToSpy("!TEST!");

		//return ((*this).*originalOperatorDivideAddr)(scalar);
		return *this;
	}
	



	typedef float(bCVector::* GetY_t)();

	inline static bCVector::GetY_t originalGetYAddr;
	inline static bCVector::GetY_t newOGetYAddr;

	__declspec(safebuffers) float GetY()
	{
		*heroPos = *this;


		count++;

		heroYs.push_back(y);

		return ((*this).*originalGetYAddr)();
	}


	inline static bCVector::GetY_t originalGetYAddr2;
	inline static bCVector::GetY_t newOGetYAddr2;

	__declspec(safebuffers) float GetY2()
	{
		//count++;

		fvar6s.push_back(y);

		return ((*this).*originalGetYAddr2)();
	}
};


/*__declspec(safebuffers) void GetFrameTimeInSeconds()
{

	count++;


	//return 0.0333333333f;
	//return 1.0f / 10.0f;
	//return 1.0f / 200.0f;
	//return 1.0f;

	DWORD timerPointer;

	__asm
	{
		call    ds: 0x2104C608
		mov     ecx, eax
		mov     timerPointer, ecx
	};

	float frametimeSeconds = *(float*)(timerPointer + 0x28);

	frameTimes.push_back(frametimeSeconds);

	//SendMessageToSpy("GetFrameTime returned " + std::to_string(frametimeSeconds));

	__asm
	{
		mov     ecx, timerPointer
		call    ds: 0x2104C604
	};
}
*/

class gCCharacterMovement_PS
{
public:


	typedef void (gCCharacterMovement_PS::* SetCurrentPosition_t)(bCVector*, bool, bool);
	inline static gCCharacterMovement_PS::SetCurrentPosition_t SetCurrentPosition;



	typedef void (gCCharacterMovement_PS::* AddToCurrentVelocity_t)(bCVector*, bool);


	inline static gCCharacterMovement_PS::AddToCurrentVelocity_t originalAddToCurrentVelocityAddr;
	inline static gCCharacterMovement_PS::AddToCurrentVelocity_t newAddToCurrentVelocityAddr;



	__declspec(safebuffers) void AddToCurrentVelocity(bCVector* vector, bool somebool)
	{
		/*vectors.push_back(*vector);

		if (vector->y >= 10000.0f)
		{
			SendMessageToSpy("!!!!!!!ABSURDNI HODNOTA!!!!!!!");

			vector->y = 1000.0f;
		}*/

		float fvar5 = *(float*)(this + 0x2F8);
		fvar5s.push_back(fvar5);


		float t2ec = *(float*)(this + 0x2EC);
		t2ecs.push_back(t2ec);

		float tc4 = *(float*)(this + 0xC4);
		tc4s.push_back(tc4);



		//SendMessageToSpy("ledgeTargetY: " + std::to_string(ledgeTargetY));

		//((*this).*originalAddToCurrentVelocityAddr)(vector, somebool);


		bCVector realPos;
		realPos.x = heroPos->x + vector->x;
		realPos.y = heroPos->y + vector->y;
		realPos.z = heroPos->z + vector->z;



		char buff[1024];
		sprintf_s(buff, "%f %f %f", realPos.x, realPos.y, realPos.z);


		SendMessageToSpy("realPos " + std::string(buff));

		
		((*this).*SetCurrentPosition)(&realPos, true, true);


		//float x = *(float*)(vector.x);
		//float y = *(float*)(vector.y);
		//float z = *(float*)(vector + 8);


		

		//char buff[1024];
		//sprintf_s(buff, "%f %f %f", vector->x, vector->y, vector->z);
		//
		//SendMessageToSpy("Vector: " + std::string(buff));
	}
};


void PrintThread()
{
	/*while (true)
	{
		Sleep(5000);

		//for (size_t i = 0; i < frameTimes.size(); i++)
		//{
		//	SendMessageToSpy("GetFrameTimeInSeconds returned " + std::to_string(frameTimes[i]));
		//}
		
		//frameTimes.clear();
		//frameTimes.reserve(50);
		
		if (count == 0)
		{
			continue;
		}

		//for (size_t i = 0; i < vectors.size(); i++)
		//{
		//	char buff[1024];
		//	sprintf_s(buff, "%f %f %f", vectors[i].x, vectors[i].y, vectors[i].z);
		//	
		//	//SendMessageToSpy("Vector " + std::to_string(i) + ": " + std::string(buff) + " | " +
		//	//				"FrameTime " + std::to_string(i) + ": " + std::to_string(frameTimes[i]) + " | " +
		//	//				"LedgeY - heroY " + std::to_string(vectors[i].y * frameTimes[i]));
		//
		//	
		//	SendMessageToSpy("Vector " + std::to_string(i) + ": " + std::string(buff));
		//}
		//
		//vectors.clear();
		//vectors.reserve(50);

		for (size_t i = 0; i < heroYs.size(); i++)
		{
			//SendMessageToSpy("HeroY: " + std::to_string(heroYs[i]) + 
			//				" | fVar6: " + std::to_string(fvar6s[i]) + 
			//				" | fVar5: " + std::to_string(fvar5s[i]));


			//SendMessageToSpy("fVar6: " + std::to_string(fvar6s[i]) +
			//				" | fVar5: " + std::to_string(fvar5s[i]) +
			//				" | this + 0x2EC: " + std::to_string(t2ecs[i]) +
			//				" | this + 0xC4: " + std::to_string(tc4s[i]));


			float fvar6 = fvar6s[i];
			float fvar5 = fvar5s[i];
			float t2ec = t2ecs[i];
			float tc4 = tc4s[i];

			float calcResult = fvar6 - fvar5 * t2ec - tc4 * 0.5 * fvar5 * fvar5;

			SendMessageToSpy(std::to_string(fvar6) + " - " + std::to_string(fvar5) + " * " + std::to_string(t2ec) + " - " + std::to_string(tc4) + " * 0.5 * " + std::to_string(fvar5) + " * " + std::to_string(fvar5) + " = " + std::to_string(calcResult) + " | " + std::to_string(heroYs[i] + results[i]));
		}

		heroYs.clear();
		heroYs.reserve(50);

		fvar6s.clear();
		fvar6s.reserve(50);

		fvar5s.clear();
		fvar5s.reserve(50);

		t2ecs.clear();
		t2ecs.reserve(50);

		tc4s.clear();
		tc4s.reserve(50);

		results.clear();
		results.reserve(50);

		SendMessageToSpy("This was called " + std::to_string(count) + " times");
		count = 0;
	}*/
}

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

//unsigned long funcAddr = (unsigned long)&GetFrameTimeInSeconds;

unsigned int t1;
unsigned int t2;
unsigned int t3;

extern "C" __declspec(dllexport) void* __stdcall ScriptInit()
{
	//unsigned long addr = 0x2018EF7A;
	//
	//unsigned long func = (unsigned long)&GetFrameTimeInSeconds;
	//unsigned long func2 = (unsigned long)&func;
	//
	//WriteProcessMemory(GetCurrentProcess(), (LPVOID)addr, &func, sizeof(func), nullptr);

	spyHandle = FindWindowA(0, "[zSpy]");


	heroPos = new bCVector();


	/*callPtr instruction = {0xFF, 0x15, (unsigned long)&funcAddr};
	WriteProcessMemory(GetCurrentProcess(), (void*)0x2018EF78, &instruction, 6, NULL);


	*/
	unsigned int aaa = 0x2004BA79;
	memcpy(&gCCharacterMovement_PS::SetCurrentPosition, &aaa, sizeof(int));


	unsigned int b = 0x2000673F;
	memcpy(&gCCharacterMovement_PS::originalAddToCurrentVelocityAddr, &b, sizeof(int));

	gCCharacterMovement_PS::newAddToCurrentVelocityAddr = &gCCharacterMovement_PS::AddToCurrentVelocity;
	unsigned int t;
	memcpy(&t, &gCCharacterMovement_PS::newAddToCurrentVelocityAddr, sizeof(int));

	call instruction2 = { 0xE8, (unsigned long)t - 0x2018EF94 - 5};
	WriteProcessMemory(GetCurrentProcess(), (void*)0x2018EF94, &instruction2, 5, NULL);
	











	unsigned int b1 = *((unsigned int*)0x2104DE58);
	memcpy(&bCVector::originalOperatorDivideAddr, &b1, sizeof(int));

	bCVector::newOperatorDivideAddr = &bCVector::operator/;
	memcpy(&t1, &bCVector::newOperatorDivideAddr, sizeof(int));

	//callPtr instruction3 = { 0xFF, 0x15, (unsigned long)&t2 };
	//WriteProcessMemory(GetCurrentProcess(), (void*)0x2018EF8B, &instruction3, 6, NULL);

	call instruction1 = { 0xE8, (unsigned long)t1 - 0x2018EF8B - 5 };
	WriteProcessMemory(GetCurrentProcess(), (void*)0x2018EF8B, &instruction1, 5, NULL);

	unsigned char asdasdasdasdasd1 = 0x90;
	WriteProcessMemory(GetCurrentProcess(), (void*)(0x2018EF8B + 5), &asdasdasdasdasd1, 1, NULL);





	unsigned int b2 = *((unsigned int*)0x2104DE68);
	memcpy(&bCVector::originalGetYAddr, &b2, sizeof(int));

	bCVector::newOGetYAddr = &bCVector::GetY;
	memcpy(&t2, &bCVector::newOGetYAddr, sizeof(int));

	call instruction = { 0xE8, (unsigned long)t2 - 0x2018EF3B - 5 };
	WriteProcessMemory(GetCurrentProcess(), (void*)0x2018EF3B, &instruction, 5, NULL);

	unsigned char asdasdasdasdasd = 0x90;
	WriteProcessMemory(GetCurrentProcess(), (void*)(0x2018EF3B + 5), &asdasdasdasdasd, 1, NULL);

	



	unsigned int b3 = *((unsigned int*)0x2104DE68);
	memcpy(&bCVector::originalGetYAddr2, &b3, sizeof(int));

	bCVector::newOGetYAddr2 = &bCVector::GetY2;
	memcpy(&t3, &bCVector::newOGetYAddr2, sizeof(int));

	call instruction3 = { 0xE8, (unsigned long)t3 - 0x2018ECCD - 5 };
	WriteProcessMemory(GetCurrentProcess(), (void*)0x2018ECCD, &instruction3, 5, NULL);

	unsigned char asdasdasdasdasd2 = 0x90;
	WriteProcessMemory(GetCurrentProcess(), (void*)(0x2018ECCD + 5), &asdasdasdasdasd2, 1, NULL);







	std::thread thread(PrintThread);

	thread.detach();


	SendMessageToSpy("test");



	return nullptr;
}
