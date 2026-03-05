// AIDA64Parser.cpp: implementation of the CAIDA64Parser class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "AIDA64Parser.h"
//////////////////////////////////////////////////////////////////////
CAIDA64Parser::CAIDA64Parser()
{
	m_strID		= "";
	m_strLabel	= "";
	m_strValue	= "";
}
//////////////////////////////////////////////////////////////////////
CAIDA64Parser::~CAIDA64Parser()
{
}
//////////////////////////////////////////////////////////////////////
#define PARSER_STATE_VAL		0
#define PARSER_STATE_TAG_HEAD	1
#define PARSER_STATE_TAG_TAIL	2
//////////////////////////////////////////////////////////////////////
BOOL CAIDA64Parser::ParseXML(LPCSTR lpText, DWORD dwContext)
{
	if (!lpText)
		//validate input text ptr
		return FALSE;

	CStringList			strValStack;
		//item value stack
	CStringList			strTagStack;
		//item tag stack

	CString	strVal	= "";
		//current item value accumulator
	CString	strTag	= "";
		//current item tag accumulator

	int		nState	= PARSER_STATE_VAL;
		//parser state
	char	symbol	= *lpText;
		//current symbol

	while (symbol)
		//loop through all symbols in the line till trailing NULL-terminator
	{
		switch (symbol)
			//process symbols depending on the current parser state
		{
		case '<':
			//process tag initiator symbol
			switch (nState)
			{
			case PARSER_STATE_VAL:
				nState	= PARSER_STATE_TAG_HEAD;
					//switch parser to tag head state
				strTag	= "";
					//reset tag name accumulator
				break;
			case PARSER_STATE_TAG_HEAD:
			case PARSER_STATE_TAG_TAIL:
				return FALSE;
					//tag initiator symbol is not allowed inside tags
			}
			break;
		case '>':
			//process tag terminator symbol
			switch (nState)
			{
			case PARSER_STATE_VAL:
				return FALSE;
					//tag terminator symbol is not allowed outside tags
			case PARSER_STATE_TAG_HEAD:

				//handle tag head 

				strTagStack.AddTail(strTag);
					//push tag accumulator
				strValStack.AddTail(strVal);
					//push value accumulator
				nState	= PARSER_STATE_VAL;
					//switch parser to value state
				strVal	= "";
					//reset value accumulator
				break;
			case PARSER_STATE_TAG_TAIL:

				//handle tag tail

				if (strTagStack.IsEmpty())
					//return error if there are no open begin tags in tag stack
					return FALSE;
				if (_stricmp(strTagStack.RemoveTail(), strTag))
					//return error if the last open begin tag in tag stack doesn't
					//match with end tag we're vurrently processing
					return FALSE;

				ParseXMLElement(strTag, strVal, strTagStack.GetCount(), dwContext);
					//element is completely declared now so we can parse it

				nState	= PARSER_STATE_VAL;
					//swicth parser to value state
				strVal	= strValStack.RemoveTail();
					//pop the previous value from value stack
				break;
			}
			break;
		case '/':
			//process tag tail initiator symbol
			switch (nState)
			{
			case PARSER_STATE_VAL:
				strVal += symbol;
					//append value accumulator if the parser is in value state now
				break;
			case PARSER_STATE_TAG_HEAD:
				if (strlen(strTag))
					return FALSE;	
					//tag tail initiator symbol is not allowed inside tag head name
				else
					nState = PARSER_STATE_TAG_TAIL;
						//switch parser to tag tail state
				break;
			case PARSER_STATE_TAG_TAIL:
					//tag tail initiator symbol is not allowed inside tag tail name
				return FALSE;
			}
			break;
		default:
			//process the rest symbols
			switch (nState)
			{
			case PARSER_STATE_VAL:
				strVal += symbol;
					//append value accumulator if the parser is in value state now
				break;
			case PARSER_STATE_TAG_HEAD:
			case PARSER_STATE_TAG_TAIL:
				strTag += symbol;
					//append tag accumulator if the parser is in tag state now
				break;
			}
			break;
		}

		symbol = *++lpText;
			//modify current symbol ptr and precache new symbol to be processed on the
			//next iteration
	}

	return TRUE;
}
//////////////////////////////////////////////////////////////////////
void CAIDA64Parser::ParseXMLElement(LPCSTR lpTag, LPCSTR lpVal, int nLevel, DWORD dwContext)
{
	switch (nLevel)
		//elements are handled differently depending on their level:

		//level 1 elements defining sensor properties are parsed first, then level 0
		//element defining whole sensor is parsed
	{
	case 0:
		//parse level 0 element defining whole sensor (at this step we've already
		//parsed and saved child level 1 elements defining sensor properties)

		if (strlen(m_strID		) &&
			strlen(m_strValue	))
			ParseSensor(lpTag, m_strID, m_strLabel, m_strValue, dwContext);
				//parse sensor if both ID and value are defined

		m_strID		= "";
			//reset sensor ID
		m_strLabel	= "";
			//reset sensor label
		m_strValue	= "";
			//reset sensor value
		break;

	case 1:
		//parse level 1 element defining sensor properties (id, label, value etc.)

		if (!_stricmp(lpTag, "id"))
			m_strID = lpVal;
				//save sensor id

		if (!_stricmp(lpTag, "label"))
			m_strLabel = lpVal;
				//save sensor label

		if (!_stricmp(lpTag, "value"))
			m_strValue = lpVal;
				//save sensor value
		break;	
	}
}
//////////////////////////////////////////////////////////////////////
