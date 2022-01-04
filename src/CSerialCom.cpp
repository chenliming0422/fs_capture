/***********************************************************************************************
* Copyright (C), 2021.Purdue University Mechanical Engineering, XTZT Lab, All rights reserved.
* @file: CSerialCom.cpp 
* @brief: 
* @author: Liming Chen
* @email: chen3496@purdue.edu
* @version: V1.0
* @date 2021-12-07
* @note
* @warning
* @todo
* @history:
***********************************************************************************************/

#include "CSerialCom.h"

/*
* @brief: constructor
* @input: none
*/
CSerialCom::CSerialCom(int portID):
m_hCom(INVALID_HANDLE_VALUE),
m_isOpen(false),
m_thread(NULL)
{
	memset(&m_ovWait, 0, sizeof(m_ovWait));
	memset(&m_ovWrite, 0, sizeof(m_ovWrite));
	memset(&m_ovRead, 0, sizeof(m_ovRead));
	memset(&m_readBuf, 0, sizeof(m_readBuf));
	m_portID = new CHAR[10];
	wsprintf(m_portID, "\\\\.\\COM%0d", portID);
	cout << "initializing port: " << m_portID << endl;
}

/*
* @brief: default destructor
* @input: none
*/
CSerialCom::~CSerialCom()
{
	close();
}

/*
* @brief: 
* @input: 
*/
bool CSerialCom::open(int baudRate)
{
	m_hCom = CreateFile(m_portID, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (INVALID_HANDLE_VALUE == m_hCom)
	{
		return false;
	}

	// set buffer size
	SetupComm(m_hCom, 4096, 4096);

	// configure serial port
	DCB dcb;
	GetCommState(m_hCom, &dcb);
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = baudRate;
	dcb.StopBits = ONESTOPBIT;
	SetCommState(m_hCom, &dcb);

	PurgeComm(m_hCom, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);

	// set timeout
	COMMTIMEOUTS ct;
	ct.ReadIntervalTimeout = 0;
	ct.ReadTotalTimeoutConstant = 5000;
	ct.ReadTotalTimeoutMultiplier = 500;
	ct.WriteTotalTimeoutConstant = 5000;
	ct.WriteTotalTimeoutMultiplier = 500;
	SetCommTimeouts(m_hCom, &ct);

	m_isOpen = true;
	return true;
}

/*
* @brief:
* @input:
*/
void CSerialCom::close()
{
	m_isOpen = false;

	if (INVALID_HANDLE_VALUE != m_hCom)
	{
		CloseHandle(m_hCom);
		m_hCom = INVALID_HANDLE_VALUE;
	}

	if (INVALID_HANDLE_VALUE != m_ovRead.hEvent)
	{
		CloseHandle(m_ovRead.hEvent);
		m_ovRead.hEvent = INVALID_HANDLE_VALUE;
	}

	if (INVALID_HANDLE_VALUE != m_ovWrite.hEvent)
	{
		CloseHandle(m_ovWrite.hEvent);
		m_ovWrite.hEvent = INVALID_HANDLE_VALUE;
	}

	if (INVALID_HANDLE_VALUE != m_ovWait.hEvent)
	{
		CloseHandle(m_ovWait.hEvent);
		m_ovWait.hEvent = INVALID_HANDLE_VALUE;
	}

	if (INVALID_HANDLE_VALUE != m_thread)
	{
		CloseHandle(m_thread);
		m_thread = INVALID_HANDLE_VALUE;
	}
}

/*
* @brief:
* @input:
*/
DWORD CSerialCom::write(LPBYTE buf, DWORD cmdLen)
{
	bool rtn = false;
	DWORD wrLen = 0;

	PurgeComm(m_hCom, PURGE_TXCLEAR | PURGE_TXABORT);

	m_ovWait.Offset = 0;

	rtn = WriteFile(m_hCom, buf, cmdLen, &wrLen, &m_ovWrite);
	if (false == rtn && GetLastError() == ERROR_IO_PENDING)
	{
		if (false == GetOverlappedResult(m_hCom, &m_ovWrite, &wrLen, true));
		{
			return 0;
		}
	}

	return wrLen;
}

/*
* @brief:
* @input:
*/
DWORD CSerialCom::read(LPBYTE buf, DWORD cmdLen)
{
	DWORD readSize = 0;
	bool rtn = false;
	DWORD rdLen = 0;

	rtn = ReadFile(m_hCom, buf, 1, &readSize, NULL);
	rdLen++;

	if (rtn = true && 1 == readSize)
	{
		DWORD error;
		COMSTAT cs = { 0 };
		int readLen = 0;

		// check unread bytes
		ClearCommError(m_hCom, &error, &cs);
		readLen = (cs.cbInQue > cmdLen) ? cmdLen : cs.cbInQue;
		if (readLen > 0)
		{
			rtn = ReadFile(m_hCom, buf + 1, readLen, &readSize, NULL);	
			if (rtn == true)
			{
				rdLen += readSize;
			}
		}
	}
	PurgeComm(m_hCom, PURGE_RXABORT | PURGE_RXCLEAR);
	return rdLen;
}

