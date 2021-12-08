/***********************************************************************************************
* Copyright (C), 2021.Purdue University Mechanical Engineering, XTZT Lab, All rights reserved.
* @file: CSerialPort.cpp 
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

#include "CSerialPort.h"

/*
* @brief: default constructor
* @input: none
*/
CSerialPort::CSerialPort(int portID):
m_hCom(INVALID_HANDLE_VALUE),
m_isOpen(false),
m_thread(NULL)
{
	memset(&m_ovWait, 0, sizeof(m_ovWait));
	memset(&m_ovWrite, 0, sizeof(m_ovWrite));
	memset(&m_ovRead, 0, sizeof(m_ovRead));
	memset(&m_readBuf, 0, sizeof(m_readBuf)); 
	wsprintf(m_portID, "\\\\.\\COM%0d", portID);
}

/*
* @brief: default destructor
* @input: none
*/
CSerialPort::~CSerialPort()
{
	closePort();
}

/*
* @brief: 
* @input: 
*/
bool CSerialPort::openPort(int baudRate, bool isSyncRead)
{
	m_isSyncRead = isSyncRead;
	if (isSyncRead == true)
	{
		m_hCom = CreateFile(m_portID, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	}
	else
	{
		m_hCom = CreateFile(m_portID, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
							FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, NULL);
	}


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
	if (isSyncRead == true)
	{
		ct.ReadIntervalTimeout = 0;
		ct.ReadTotalTimeoutConstant = 5000;
		ct.ReadTotalTimeoutMultiplier = 500;
	}
	else
	{
		ct.ReadIntervalTimeout = MAXDWORD;
		ct.ReadTotalTimeoutConstant = 0;
		ct.ReadTotalTimeoutMultiplier = 0;
	}
	ct.WriteTotalTimeoutConstant = 5000;
	ct.WriteTotalTimeoutMultiplier = 500;
	SetCommTimeouts(m_hCom, &ct);

	// set event
	if (isSyncRead == false)
	{
		m_ovRead.hEvent = CreateEvent(NULL, false, false, NULL);
		m_ovWrite.hEvent = CreateEvent(NULL, false, false, NULL);
		m_ovWait.hEvent = CreateEvent(NULL, false, false, NULL);

		// create read thread
		m_thread = (HANDLE)_beginthreadex(NULL, 0, &CSerialPort::recv, this, 0, NULL);
	}

	m_isOpen = true;
	return true;
}

/*
* @brief:
* @input:
*/
void CSerialPort::closePort()
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
bool CSerialPort::writePort(LPBYTE buf, DWORD cmdLen, DWORD* wrLen)
{
	bool rtn = false;
	*wrLen = 0;

	PurgeComm(m_hCom, PURGE_TXCLEAR | PURGE_TXABORT);

	m_ovWait.Offset = 0;

	rtn = WriteFile(m_hCom, buf, cmdLen, wrLen, &m_ovWrite);
	if (false == rtn && GetLastError() == ERROR_IO_PENDING)
	{
		if (false == GetOverlappedResult(m_hCom, &m_ovWrite, wrLen, true));
		{
			return false;
		}
	}

	return rtn != false;
}

/*
* @brief:
* @input:
*/
unsigned int __stdcall CSerialPort::recv(void* LPParam)
{
	CSerialPort* obj = static_cast<CSerialPort*>(LPParam);
	DWORD waitEvent = 0;
	bool status = false;
	DWORD error;
	COMSTAT cs = { 0 };
	obj->m_validReadBytes = 0;

	while (obj->m_isOpen)
	{
		waitEvent = 0;
		obj->m_ovWait.Offset = 0;

		// wait event
		status = WaitCommEvent(obj->m_hCom, &waitEvent, &obj->m_ovWait);
		if (false == status && GetLastError() == ERROR_IO_PENDING)
		{
			status = GetOverlappedResult(obj->m_hCom, &obj->m_ovWait, &obj->m_validReadBytes, true);
		}
		ClearCommError(obj->m_hCom, &error, &cs);

		// detect event and buffer has data
		if (true == status && waitEvent&EV_RXCHAR && cs.cbInQue > 0)
		{
			obj->m_validReadBytes = 0;
			obj->m_ovRead.Offset = 0;
			memset(obj->m_readBuf, 0, sizeof(obj->m_readBuf));

			// the data has reached the buffer
			status = ReadFile(obj->m_hCom, obj->m_readBuf, sizeof(obj->m_readBuf), &obj->m_validReadBytes, &obj->m_ovRead);
			if (status == false)
			{
				memset(obj->m_readBuf, 0, sizeof(obj->m_readBuf));
				obj->m_validReadBytes = 0;
			}
			PurgeComm(obj->m_hCom, PURGE_RXABORT | PURGE_RXCLEAR);
		}
	}
	return 0;
}

/*
* @brief:
* @input:
*/
bool CSerialPort::readPortSync(LPBYTE buf, DWORD cmdLen, DWORD* rdLen)
{
	DWORD readSize = 0;
	bool rtn = false;

	rtn = ReadFile(m_hCom, buf, 1, &readSize, NULL);

	if (rtn = true && 1 == readSize)
	{
		DWORD error;
		COMSTAT cs = { 0 };
		int readLen = 0;

		// check unread bytes
		ClearCommError(m_hCom, &error, &cs);
		readLen = (cs.cbInQue > cmdLen - 1) ? cmdLen - 1 : cs.cbInQue;
		if (readLen > 0)
		{
			rtn = ReadFile(m_hCom, buf + 1, readLen, &readSize, NULL);
			*rdLen = 0;
			if (rtn == true)
			{
				*rdLen = readSize + 1;
			}
		}
	}
	PurgeComm(m_hCom, PURGE_RXABORT | PURGE_RXCLEAR);
	return rtn != false;
}

