#pragma once
/***********************************************************************************************
* Copyright (C), 2021.Purdue University Mechanical Engineering, XTZT Lab, All rights reserved.
* @file: CSerialPort.h 
* @brief: Header file for CSerialPort.cpp
* @author: Liming Chen
* @email: chen3496@purdue.edu
* @version: V1.0
* @date: 2021-12-07
* @note
* @warning
* @todo
* @history:
***********************************************************************************************/

#include <windows.h>
#include <process.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

class CSerialPort
{
private:
    LPSTR m_portID;
	bool m_isSyncRead;
    HANDLE m_hCom;
    OVERLAPPED m_ovWrite;
    OVERLAPPED m_ovRead;
    OVERLAPPED m_ovWait;
    volatile bool m_isOpen;
    HANDLE m_thread;

	DWORD m_validReadBytes;
	BYTE m_readBuf[4096];

// constructor and destructor
public:
    CSerialPort(int portID);
    ~CSerialPort();

public:
    bool openPort(int baudRate, bool isSyncRead);
    void closePort();

    // write
    bool writePort(LPBYTE buf, DWORD cmdLen, DWORD* wrLen);

	// sync read
	bool readPortSync(LPBYTE buf, DWORD cmdLen, DWORD* rdLen);

    // async read thread
    static unsigned int __stdcall recv(void *);
	int getReadBuf(BYTE* readData);
};