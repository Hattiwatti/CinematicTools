#include "Offsets.h"
#include <Psapi.h>
#include "Tools\Log\Log.h"

#define READABLE (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY)

bool Offsets::m_initialized = false;
vector<Signature*> Offsets::m_sigs;

std::map<std::string, __int64> Offsets::m_hardcoded = map_list_of
("OFFSET_CAMERAHOOK", 0x1439A51B0)
("OFFSET_MOUSEHOOK", 0x1431817A0)
("OFFSET_GAMECONTROLS", 0x143184F70)
("OFFSET_TIMESCALE", 0x1427D9010)
("OFFSET_FREEZENOP1", 0x1433A477E)
("OFFSET_FREEZENOP2", 0x0)
("OFFSET_FREEZENOP3", 0x1437D35E2)
("OFFSET_FREEZENOP3", 0x1437D35C7);

bool DataCompare(BYTE* pData, BYTE* bSig, char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bSig)
	{
		if (*szMask == 'x' && *pData != *bSig)
			return false;
	}
	return (*szMask) == 0;
}

BYTE* FindPattern(BYTE* dwAddress, __int64 dwSize, BYTE* pbSig, char* szMask)
{
	register BYTE bFirstByte = *(BYTE*)pbSig;

	__int64 length = (__int64)dwAddress + dwSize - strlen(szMask);

	for (register __int64 i = (__int64)dwAddress; i < length; i += 4) // might run faster with 8 bytes but I am too lazy
	{
		unsigned x = *(unsigned*)(i);

		if ((x & 0xFF) == bFirstByte)
			if (DataCompare(reinterpret_cast<BYTE*>(i), pbSig, szMask))
				return reinterpret_cast<BYTE*>(i);

		if ((x & 0xFF00) >> 8 == bFirstByte)
			if (DataCompare(reinterpret_cast<BYTE*>(i + 1), pbSig, szMask))
				return reinterpret_cast<BYTE*>(i + 1);

		if ((x & 0xFF0000) >> 16 == bFirstByte)
			if (DataCompare(reinterpret_cast<BYTE*>(i + 2), pbSig, szMask))
				return reinterpret_cast<BYTE*>(i + 2);

		if ((x & 0xFF000000) >> 24 == bFirstByte)
			if (DataCompare(reinterpret_cast<BYTE*>(i + 3), pbSig, szMask))
				return reinterpret_cast<BYTE*>(i + 3);
	}
	return 0;
}

BYTE CharToByte(char c)
{
	BYTE b;
	sscanf(&c, "%hhx", &b);
	return b;
}

Signature::Signature(string name, string pattern, int offset)
{
	*this = Signature(name, pattern);
	this->m_offset = offset;
	this->m_isFunction = true;
}

Signature::Signature(string name, string pattern)
{
	this->name = name;
	this->m_isFunction = true;

	int Index = 0;
	char* pChar = &pattern[0];

	m_pattern = (PBYTE)calloc(0x1, pattern.size());
	m_mask = (PCHAR)calloc(0x1, pattern.size());

	while (*pChar)
	{
		if (*pChar == ' ')
		{
			pChar++;
			continue;
		}

		if (*pChar == ']')
		{
			pChar++;
			m_size = Index - m_offset;
			continue;
		}

		if (*pChar == '[')
		{
			this->m_isFunction = false;
			pChar++;
			m_offset = Index;
			continue;
		}

		if (*pChar == '?')
		{
			m_mask[Index++] += '?';
			pChar += 2;
			continue;
		}

		m_mask[Index] = 'x';
		m_pattern[Index++] = (CharToByte(pChar[0]) << 4) + CharToByte(pChar[1]);
		pChar += 2;
	}
}

void Offsets::Scan()
{
	Log::Write("Scanning for offsets...");

	HMODULE handle = GetModuleHandleA("ROTTR.exe");
	MODULEINFO info;
	bool success = GetModuleInformation(GetCurrentProcess(), handle, &info, sizeof(MODULEINFO));

	for (vector<Signature*>::iterator itr = m_sigs.begin(); itr != m_sigs.end(); ++itr)
	{
		BYTE* result = FindPattern((BYTE*)info.lpBaseOfDll, (__int64)info.SizeOfImage, (*itr)->m_pattern, (*itr)->m_mask);
		if (result)
		{
			if ((*itr)->m_isFunction)
				(*itr)->result = (INT64)result + (*itr)->m_offset;
			else
			{
				int* relative = (int*)((INT64)result + (INT64)(*itr)->m_offset);
				(*itr)->result = ((INT64)result + (INT64)(*itr)->m_offset + 0x4 + *relative);
			}
		}
	}

	bool AllFound = true;

	fstream file;
	//file.open("Offsets.txt", std::fstream::in | std::fstream::out | std::fstream::trunc);
	for (vector<Signature*>::iterator itr = m_sigs.begin(); itr != m_sigs.end(); ++itr)
	{
		Log::Write((*itr)->name + " 0x" + Log::hexa((*itr)->result));
		//file << "(\"" + (*itr)->name + "\", \t\t0x" + Log::hexa((*itr)->result) + " )\n";
		if (!(*itr)->result)
		{
			Log::WriteError("Could not find " + (*itr)->name);
			AllFound = false;
		}
	}

	if (AllFound)
		Log::WriteOK("All offsets found");

	printf("\n");
}

INT64 Offsets::GetOffset(string name)
{
	for (vector<Signature*>::iterator itr = m_sigs.begin(); itr != m_sigs.end(); ++itr)
	{
		if ((*itr)->name == name)
		{
			if (!(*itr)->result)
				break;
			return (*itr)->result;
		}
	}

	std::map<std::string, __int64>::iterator result = m_hardcoded.find(name);
	if (result != m_hardcoded.end())
	{
		return result->second;
	}

	//Log::WriteError("Couldn't find " + name);
	return NULL;
}

void Offsets::Init()
{
	m_sigs.push_back(new Signature("OFFSET_CAMERAHOOK", "48 89 E0 48 89 58 10 48 89 68 18 57 41 56 41 57 48 81 EC ?? ?? ?? ?? 0F 29 78 C8", 0));
	m_sigs.push_back(new Signature("OFFSET_MOUSEHOOK", "48 89 E0 44 88 40 18 57", 0));

	m_sigs.push_back(new Signature("OFFSET_GAMECONTROLS", "49 89 E3 57 41 55", 0));

	m_sigs.push_back(new Signature("OFFSET_TIMESCALE", "F3 0F 10 2D [ ?? ?? ?? ?? ] FF 05 ?? ?? ?? ??"));
	m_sigs.push_back(new Signature("OFFSET_FREEZENOP1", "48 89 86 ?? ?? ?? ?? 48 8B 49 08", 0));
	m_sigs.push_back(new Signature("OFFSET_FREEZENOP2", "89 1F E9 ?? ?? ?? ?? 81 83 FE 04 0F 85 CF 00 00 00", 0));
	m_sigs.push_back(new Signature("OFFSET_FREEZENOP3", "F3 0F 11 83 ?? ?? ?? ?? 48 8B 43 20", 0));
	m_sigs.push_back(new Signature("OFFSET_FREEZENOP3", "89 83 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 89 C1 E8 ?? ?? ?? ?? F3 0F 59 05", 0));
	Scan();
}