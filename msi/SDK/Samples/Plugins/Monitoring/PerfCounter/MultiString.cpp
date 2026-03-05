#include "stdafx.h"
#include "MultiString.h"
//////////////////////////////////////////////////////////////////////
CMultiString::CMultiString(LPCSTR lpmsz)
{
	Init(lpmsz);
}
//////////////////////////////////////////////////////////////////////
CMultiString::CMultiString(LPCSTR lpmsz, DWORD size)
{
	Init(lpmsz, size);
}
//////////////////////////////////////////////////////////////////////
CMultiString::~CMultiString()
{
}
//////////////////////////////////////////////////////////////////////
void CMultiString::Init(LPCSTR lpmsz)
{
	m_lpmsz = lpmsz;
}
//////////////////////////////////////////////////////////////////////
void CMultiString::Init(LPCSTR lpmsz, DWORD size)
{
	if (lpmsz && size)
	{
		while (!*lpmsz && size)
		{
			lpmsz++;

			size--;
		}
	}

	if (size)
		m_lpmsz = lpmsz;
	else
		m_lpmsz	= NULL;
}
//////////////////////////////////////////////////////////////////////
LPCSTR CMultiString::GetNext()
{
	if (m_lpmsz)
	{
		int len = strlen(m_lpmsz);

		if (len)
		{
			LPCSTR lpNext = m_lpmsz;

			m_lpmsz += len + 1;

			return lpNext;
		}
		else
			m_lpmsz = NULL;
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////
LPCSTR CMultiString::GetIndex(DWORD dwIndex)
{
	LPCSTR lpName = GetNext();

	while (lpName)
	{
		if (!dwIndex)
			return lpName;

		lpName = GetNext();

		dwIndex--;
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////
DWORD CMultiString::FindIndexInMap(LPCSTR lpSample)
{
	LPCSTR lpIndex	= GetNext();
	LPCSTR lpName	= GetNext();

	while (lpIndex && lpName)
	{
		if (!_stricmp(lpName, lpSample))
		{
			DWORD dwIndex;

			if (sscanf_s(lpIndex, "%d", &dwIndex) == 1)
				return dwIndex;
		}

		lpIndex	= GetNext();
		lpName	= GetNext();
	}

	return 0xFFFFFFFF;
}
//////////////////////////////////////////////////////////////////////
LPCSTR CMultiString::FindNameInMap(DWORD dwSample)
{
	LPCSTR lpIndex	= GetNext();
	LPCSTR lpName	= GetNext();

	while (lpIndex && lpName)
	{
		DWORD dwIndex;

		if (sscanf_s(lpIndex, "%d", &dwIndex) == 1)
		{
			if (dwIndex == dwSample)
				return lpName;
		}

		lpIndex	= GetNext();
		lpName	= GetNext();
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////
