#pragma once
/***********************************************************************************************
* Copyright (C), 2021.Purdue University Mechanical Engineering, XTZT Lab, All rights reserved.
* @file: CETLCtrl.h
* @brief: Header file for CETLCtrl.cpp
* @author: Liming Chen
* @email: chen3496@purdue.edu
* @version: V1.0
* @date: 2021-12-07
* @note: The details can be found in Optotune Electrical Lens Driver 4 manual: https://www.optotune.com/controllers
* @warning
* @todo
* @history:
***********************************************************************************************/

#include "CSerialCom.h"
#include <vector>
#include <math.h>
#include <assert.h>

// lens type definition
#define EL_16_40_TC_VIS_5D  0
#define EL_16_40_TC_VIS_20D 1

// control mode definition
#define EL_SINU 0
#define EL_SQUA 1
#define EL_DC   2
#define EL_TRIA 3
#define EL_CTRL 4

class CETLCtrl
{
private:
	int m_type;
	int m_portID;
	int m_baudRate;
	CSerialCom* m_serialCom;
	bool m_connect;
	bool m_sinMode;
	float m_maxCurrent;
	float m_minCurrent;
	float m_calibCurrent;
	uint16_t m_crcTable[256];

public:
	CETLCtrl(int portID, int type);
	~CETLCtrl();

private:
	bool addCRC(vector<uint8_t>& cmd);
	uint16_t computeCheckSum(vector<uint8_t>& cmd);
	void generateTable();

public:
	bool connect();
	bool disconnect();
	bool setSinMode();
	bool setCurrent(float current);
	bool setMinMax(float upper, float lower);
	bool setFreq(float freq);
	//bool setLimit(float upper, float lower);
	//bool readLimit();
};