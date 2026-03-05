#pragma once
//////////////////////////////////////////////////////////////////////
class CMultiString
{
public:
	void	Init(LPCSTR lpmsz);
	void	Init(LPCSTR lpmsz, DWORD size);

	LPCSTR	GetNext();
	LPCSTR	GetIndex(DWORD dwIndex);
	DWORD	FindIndexInMap(LPCSTR lpSample);
	LPCSTR	FindNameInMap(DWORD dwSample);

	CMultiString(LPCSTR lpmsz);
	CMultiString(LPCSTR lpmsz, DWORD size);
	~CMultiString();

private:
	LPCSTR m_lpmsz;
};
//////////////////////////////////////////////////////////////////////
