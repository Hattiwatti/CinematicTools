#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include "boost\assign.hpp"

using namespace std;
using namespace boost::assign;

class Signature
{
public:
	Signature(string, string, int);
	Signature(string, string);

	PBYTE m_pattern;
	PCHAR m_mask;

	int m_isFunction;

	int m_offset;
	int m_size;

	string name;
	INT64 result;
};

class Offsets
{
public:
	static void Init();
	static INT64 GetOffset(string name);

	static void Scan();

	static vector<Signature*> m_sigs;
	static bool m_initialized;
	static std::map<std::string, __int64> m_hardcoded;
};