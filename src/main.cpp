////////////////////////////////////////////////////////////////////////////////////////
//
// This code is for image capturing in focal stack sturctured-light 3D imaging system
//
// Copyright(c) 2022 XYZT Lab, Purdue University.
// Author： Liming Chen
// Date: 2022-01-04
// Contact: chen3496@purdue.edu
//
///////////////////////////////////////////////////////////////////////////////////////
#include <direct.h>
#include <io.h>
#include <math.h>
#include <iostream>
#include <string>
#include <vector>

//OpenCV library
#include "opencv2/core.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

//Project
#include "CSerialCom.h"
#include "CETLCtrl.h"
#include "CCameraDriver.h"

using namespace std;
using namespace cv;

int main()
{
	float current[] = {1, 2, 3};

	// connect ETL
	CETLCtrl etl(6, EL_16_40_TC_VIS_5D);
	etl.connect();

	// init camera
	unsigned int cameraSerialNo = 16238113;
	int imageWidth = 1280;
	int imageHeight = 1000;
	bool isSaveFringe = true;
	bool isHardwareTrigger = true;
	float frameRate = 15.0f;
	int frameNum = 32;
	float expTime = 9.9f;
	CCameraDriver camera(imageWidth, imageHeight);
	camera.initSystem(cameraSerialNo, frameRate, frameNum, isHardwareTrigger);

	// create save folder
	int poseID = 1;
	string folderRoot = "../../data";
	string folderPose;
	if (isSaveFringe)
	{
		string folderPose = folderRoot + "/" + to_string(poseID);
		if (_access(folderPose.c_str(), 0) == -1)
		{
			mkdir(folderPose.c_str());
		}	
	}

	// capture images
	for (int i = 0; i < sizeof(current)/sizeof(float); i++)
	{
		string currentDir = folderPose + "/" + to_string(current[i]) + "mA";
		if (_access(currentDir.c_str(), 0) == -1)
		{
			mkdir(currentDir.c_str());
		}

		etl.setCurrent(current[i]);
		camera.grabPos(expTime, currentDir, isSaveFringe);
	}


	// close
	etl.disconnect();
	return 0;
}