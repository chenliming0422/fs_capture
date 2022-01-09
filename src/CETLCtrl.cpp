/***********************************************************************************************
* Copyright (C), 2021.Purdue University Mechanical Engineering, XTZT Lab, All rights reserved.
* @file: CETLCtrl.cpp
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

#include "CETLCtrl.h"

/*
* @brief: constructor
* @input: 
*/
CETLCtrl::CETLCtrl(int portID, int type)
{
	m_type = type;
	m_portID = portID;
	m_serialCom = new CSerialCom(portID);
	m_baudRate = 115200;
	m_connect = false;
	m_sinMode = false;
	m_calibCurrent = 292.84;
	generateTable();
}

/*
* @brief: constructor
* @input: none
*/
CETLCtrl::~CETLCtrl()
{
	delete m_serialCom;
	m_connect = false;
}

/*
* @brief: 
* @input:
*/
bool CETLCtrl::connect()
{
	m_serialCom->open(m_baudRate);
	cout << "Connecting ETL..." << endl;
	unsigned char connectCmd[] = "Start";
	int wrLen = m_serialCom->write(connectCmd, sizeof(connectCmd)-1);
	if (wrLen == sizeof(connectCmd)-1)
	{
		unsigned char rdCmd[8];
		int rdLen = m_serialCom->read(rdCmd, sizeof(rdCmd)-1);
		if (rdLen == sizeof(rdCmd)-1)
		{
			if (memcmp(rdCmd, "Ready\r\n", sizeof(rdCmd)-1) == 0)
			{
				m_connect = true;
				cout << "success!" << endl;
				return true;
			}
			else
			{
				rdCmd[7] = '\0';
				cout << "fail! Received a wrong response from the ETL, received:" << rdCmd << endl;
				m_connect = false;
				return false;
			}
		}
		else
		{
			m_connect = false;
			cout << "fail! The size of the received response is wrong" << endl;
		}
	}
	else
	{
		cout << "fail! Cannot send connect cmd, please check the serial port." << endl;
		m_connect = false;
		return false;
	}
}

/*
* @brief:
* @input:
*/
bool CETLCtrl::disconnect()
{
	m_serialCom->close();
	m_connect = false;
	return true;
}

/*
* @brief:
* @input:
*/
bool CETLCtrl::setSinMode()
{
	// if the ETL is not connected, return immediately
	if (m_connect == false)
	{
		return false;
	}

	// generate command
	vector<uint8_t> cmd = { 'M', 'w', 'S', 'A'};

	// add CRC and send command
	if (true == addCRC(cmd))
	{
		uint8_t* pCommand = new uint8_t[cmd.size()];
		std::copy(cmd.begin(), cmd.end(), pCommand);
		m_serialCom->write(pCommand, cmd.size());
		delete[] pCommand;
		uint8_t recv[8];
		m_serialCom->read(recv, 7);
		if (memcmp(recv, "MSA", 3) == 0)
		{
			m_sinMode = true;
			cout << "function setSinMode(): success" << endl;
			return true;
		}
		else
		{
			m_sinMode = false;
			cout << "function setSinMode() : received a wrong response" << endl;
			return false;
		}
	}
	else
	{
		cout << "function setSinMode() : fail to add CRC" << endl;
		m_sinMode = false;
		return false;
	}
}

/*
* @brief:
* @input:
*/
bool CETLCtrl::setCurrent(float current)
{
	// if the ETL is not connected, return immediately
	if (m_connect == false)
	{
		return false;
	}

	// generate the command
	vector<uint8_t> cmd = { 'A', 'w' };
	int16_t setNumber = (int16_t)round((current / m_calibCurrent) * 4096);

	cmd.push_back(static_cast<uint8_t>(setNumber >> 8));
	cmd.push_back(static_cast<uint8_t>(setNumber & 0xFF));

	// send the commands
	if (true == addCRC(cmd))
	{
		uint8_t* pCommand = new uint8_t[cmd.size()];
		std::copy(cmd.begin(), cmd.end(), pCommand);
		m_serialCom->write(pCommand, cmd.size());
		delete[] pCommand;
		uint8_t recv[8];
		if (0 != m_serialCom->read(recv, 7))
		{
			cout << "function setCurrent() : CRC error when set current" << endl;
			return false;
		}
		else
		{
			cout << "function setCurrent() : set current to " << current << "mA" << endl;
			return true;
		}
	}
	else
	{
		cout << "function setCurrent() : fail to add CRC" << endl;
		return false;
	}
}

/*
* @brief:
* @input:
*/
bool CETLCtrl::setMinMax(float upper, float lower)
{
	// if the ETL is not connected, return immediately
	if (m_connect == false || m_sinMode == false)
	{
		return false;
	}

	float max = upper;
	float min = lower;
	if (lower > upper)
	{
		max = lower;
		min = upper;
	}

	// generate the commands
	vector<uint8_t> minCmd = { 'P', 'w', 'L', 'A' };
	vector<uint8_t> maxCmd = { 'P', 'w', 'U', 'A' };

	int16_t minNumber = (int16_t)round((min / m_calibCurrent) * 4096);
	int16_t maxNumber = (int16_t)round((max / m_calibCurrent) * 4096);
	
	minCmd.push_back(static_cast<uint8_t>(minNumber >> 8));
	minCmd.push_back(static_cast<uint8_t>(minNumber & 0xFF));
	minCmd.push_back(0x00);
	minCmd.push_back(0x00);


	maxCmd.push_back(static_cast<uint8_t>(maxNumber >> 8));
	maxCmd.push_back(static_cast<uint8_t>(maxNumber & 0xFF));
	maxCmd.push_back(0x00);
	maxCmd.push_back(0x00);

	// send the commands
	if (true == addCRC(minCmd))
	{
		uint8_t* pCommand = new uint8_t[minCmd.size()];
		std::copy(minCmd.begin(), minCmd.end(), pCommand);
		m_serialCom->write(pCommand, minCmd.size());
		delete[] pCommand;
		cout << "function setMinMax() : set min to " << min << "mA" << endl;
	}
	else
	{
		cout << "function setMinMax() : fail to add CRC" << endl;
		return false;
	}

	if (true == addCRC(maxCmd))
	{
		uint8_t* pCommand = new uint8_t[maxCmd.size()];
		std::copy(maxCmd.begin(), maxCmd.end(), pCommand);
		m_serialCom->write(pCommand, maxCmd.size());
		delete[] pCommand;
		cout << "function setMinMax() : set max to " << max << "mA" << endl;
	}
	else
	{
		cout << "function setMinMax() : fail to add CRC" << endl;
		return false;
	}

	return true;
}

/*
* @brief:
* @input:
*/
bool CETLCtrl::setFreq(float freq)
{
	// if the ETL is not connected, return immediately
	if (m_connect == false || m_sinMode == false)
	{
		return false;
	}

	// generate the command
	vector<uint8_t> cmd = { 'P', 'w', 'F', 'A' };
	uint32_t freqNumber = static_cast<uint32_t>(freq * 1000);
	cmd.push_back(static_cast<uint8_t>(freqNumber >> 24));
	cmd.push_back(static_cast<uint8_t>((freqNumber >> 16) & 0xFF));
	cmd.push_back(static_cast<uint8_t>((freqNumber >> 8) & 0xFF));
	cmd.push_back(static_cast<uint8_t>(freqNumber & 0xFF));

	// send the command
	if (true == addCRC(cmd))
	{
		uint8_t* pCommand = new uint8_t[cmd.size()];
		std::copy(cmd.begin(), cmd.end(), pCommand);
		m_serialCom->write(pCommand, cmd.size());
		delete[] pCommand;
		cout << "function setFreq() : set frequency to " << freq <<"Hz" << endl;
	}
	else
	{
		cout << "function setFreq() : fail to add CRC" << endl;
		return false;
	}

	return true;
}


/*
* @brief:
* @input:
*/
/*bool CETLCtrl::setLimit(float upper, float lower)
{
	assert(m_connect == true);


}*/

/*
* @brief:
* @input:
*/
/*bool CETLCtrl::readLimit()
{
	assert(m_connect == true);
}*/


/*
* @brief:
* @input:
*/
void CETLCtrl::generateTable()
{
	uint16_t value;
	uint16_t temp;
	const uint16_t polynomial = 0xA001;
	for (int i = 0; i < 256; i++)
	{
		value = 0;
		temp = i;
		for (int j = 0; j < 8; j++)
		{
			if (((value ^ temp) & 0x0001) != 0)
			{
				value = static_cast<uint16_t>(value >> 1) ^ polynomial;
			}
			else
			{
				value >>= 1;
			}
			temp >>= 1;
		}
		m_crcTable[i] = value;
	}
}

/*
* @brief:
* @input:
*/
uint16_t CETLCtrl::computeCheckSum(vector<uint8_t>& cmd)
{
	uint16_t crc = 0;
	for (int i = 0; i < cmd.size(); i++)
	{
		uint8_t index = static_cast<uint8_t>(crc ^ cmd[i]);
		crc = static_cast<uint16_t>((crc >> 8) ^ m_crcTable[index]);
	}
	return crc;
}

/*
* @brief:
* @input:
*/
bool CETLCtrl::addCRC(vector<uint8_t>& cmd)
{
	uint16_t crc = computeCheckSum(cmd);
	cmd.push_back(static_cast<uint8_t>(crc & 0xFF));
	cmd.push_back(static_cast<uint8_t>(crc >> 8));

	// check if calculation is correct (should equal 0)
	uint16_t check = computeCheckSum(cmd);
	if (check == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
