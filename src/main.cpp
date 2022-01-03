#include <windows.h>
#include <math.h>
#include <iostream>
#include <vector>

//OpenCV library
#include "opencv2/core.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

//Project
#include "include/CSerialCom.h"
#include "include/CETLCtrl.h"

using namespace std;
using namespace cv;

int main()
{
	/*
	CSerialCom serial(7);
	serial.open(115200);
	BYTE cmd[] = "Start";
	int wrLen = serial.write(cmd, sizeof(cmd)-1);
	cout << wrLen << endl;
	BYTE rdcmd[8];
	int rdlen = serial.read(rdcmd, sizeof(rdcmd));
	cout <<  rdlen << rdcmd << endl;
	*/
	CETLCtrl etl(7, EL_16_40_TC_VIS_5D);
	etl.connect();
}