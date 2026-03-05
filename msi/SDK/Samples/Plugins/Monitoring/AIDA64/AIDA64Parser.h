// AIDA64Parser.h: interface for the CAIDA64Parser class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////
class CAIDA64Parser
{
public:
	BOOL ParseXML(LPCSTR lpText, DWORD dwContext = 0);
	void ParseXMLElement(LPCSTR lpTag, LPCSTR lpVal, int nLevel, DWORD dwContext);

	virtual void ParseSensor(LPCSTR lpType, LPCSTR lpID, LPCSTR lpLabel, LPCSTR lpValue, DWORD dwContext) = 0;

	CAIDA64Parser();
	virtual ~CAIDA64Parser();

protected:
	CString	m_strID;
	CString	m_strLabel;
	CString m_strValue;
};
//////////////////////////////////////////////////////////////////////
