#pragma once
/***********************************************************************************************
* Copyright (C), 2021.Purdue University Mechanical Engineering, XTZT Lab, All rights reserved.
* @file: CETLCtrl.h
* @brief: Header file for CETLCtrl.cpp
* @author: Liming Chen
* @email: chen3496@purdue.edu
* @version: V1.0
* @date: 2021-12-07
* @note
* @warning
* @todo
* @history:
***********************************************************************************************/

#include "CSerialPort.h"

class CETLCtrl
{
private:
	DWORD32 m_portID;
	CSerialPort* m_serial;
	float m_maxCurrent;
	float m_minCurrent;

public:
	CETLCtrl(DWORD32 portID, string type);
	~CETLCtrl();

public:
	bool connect();
	bool disconnect();
	bool setMode(string mode);
	bool setCurrent(float current);
};