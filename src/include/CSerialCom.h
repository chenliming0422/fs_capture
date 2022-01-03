#pragma once
/***********************************************************************************************
* Copyright (C), 2021.Purdue University Mechanical Engineering, XTZT Lab, All rights reserved.
* @file: CSerialCom.h 
* @brief: Header file for CSerialCom.cpp
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

class CSerialCom
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

public:
	DWORD m_validReadBytes;
	BYTE m_readBuf[4096];

// constructor and destructor
public:
    CSerialCom(int portID);
    ~CSerialCom();

public:
    bool open(int baudRate);
    void close();
    // write
    DWORD write(LPBYTE buf, DWORD cmdLen);
	// sync read
	DWORD read(LPBYTE buf, DWORD cmdLen);
};