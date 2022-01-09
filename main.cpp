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
	float current[] = {-130.00, -131.00, -132.00, -133.00, -134.00, -135.00, 
		               -136.00, -137.00, -138.00, -139.00, -140.00, -141.00};

	// connect ETL
	CETLCtrl etl_camera(5, EL_16_40_TC_VIS_5D);
	etl_camera.connect();

	CETLCtrl etl_proj(4, EL_16_40_TC_VIS_5D);
	etl_proj.connect();
	etl_proj.setCurrent(26.53);


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
	if (_access(folderRoot.c_str(), 0) == -1)
	{
		_mkdir(folderRoot.c_str());
	}

	string folderPose;
	if (isSaveFringe)
	{
		folderPose = folderRoot + "/" + to_string(poseID);
		if (_access(folderPose.c_str(), 0) == -1)
		{
			_mkdir(folderPose.c_str());
		}	
	}

	// capture images
	for (int i = 0; i < sizeof(current)/sizeof(float); i++)
	{
		string currentName = to_string(current[i]);
		string currentNameFix = currentName.substr(0, currentName.find(".") + 3);
		string currentDir = folderPose + "/" + currentNameFix + "mA";
		if (_access(currentDir.c_str(), 0) == -1)
		{
			_mkdir(currentDir.c_str());
		}

		etl_camera.setCurrent(current[i]);
		camera.grabPos(expTime, currentDir, isSaveFringe);
	}


	// close
	etl_camera.disconnect();
	etl_proj.disconnect();
	return 0;
}