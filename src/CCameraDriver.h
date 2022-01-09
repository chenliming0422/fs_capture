#pragma once
#include <windows.h>
#include <iostream>
#include <vector>

#include "pointGreyCapture.h"

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

class CCameraDriver
{

public:
	CCameraDriver(int imgWidth, int imgHeight);
	~CCameraDriver();

private:
	unsigned int m_cameraSerialNo;
	Size m_cameraRes;
	int m_cameraSize;
	bool m_isHardwareTrigger;
	float m_frameRate;
	int m_frameNum;
	bool m_systemInit;
	pointGreyCapture m_grab;
	unsigned char ** m_captureImage;

public:
	unsigned char ** m_storeImage;


private:
	bool drawLiveImage(string windowName, Mat inputImage);
	void livePreview(string windowName, float expTime);
	int findBlackImageID();
	bool saveRawImage(string floderDir);

public:
	void initSystem(unsigned int cameraSerialNo, float frameRate, int frameNum, bool isHardwareTrigger);
	int grabImageSet(float expTime);
	void grabPos(float expTime, string folderDir, bool saveRawImage = false, bool isLivePreview = true);
};