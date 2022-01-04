////////////////////////////////////////////////////////////////////////////////////////
//
// This class is used to capture images for structured light system
//
// Copyright(c) 2020 XYZT Lab, Purdue University.
// Author： Liming Chen
// Date: Apr. 27. 2021
// Contact: chen3496@purdue.edu
//
///////////////////////////////////////////////////////////////////////////////////////

#include "CCameraDriver.h"

/*
*
*/
CCameraDriver::CCameraDriver(int imgWidth, int imgHeight)
{
	m_systemInit = false;
	m_cameraRes.width = imgWidth;
	m_cameraRes.height = imgHeight;
	m_cameraSize = imgWidth * imgHeight;
}

/*
*
*/
CCameraDriver::~CCameraDriver()
{
	for (int i = 0; i < m_frameNum; i++)
	{
		delete[] m_storeImage[i];
		delete[] m_captureImage[i];
		delete[] m_captureImage[i + m_frameNum];
	}
	delete m_storeImage;
	delete m_captureImage;
}

/*
*
*/
void CCameraDriver::initSystem(unsigned int cameraSerialNo, float frameRate, int frameNum, bool isHardwareTrigger)
{
	m_cameraSerialNo = cameraSerialNo;
	m_frameRate = frameRate;
	m_frameNum = frameNum;
	m_isHardwareTrigger = isHardwareTrigger;

	m_storeImage = new unsigned char*[m_frameNum];
	m_captureImage = new unsigned char*[2 * m_frameNum];
	for (int i = 0; i < m_frameNum; i++)
	{
		m_storeImage[i] = new unsigned char[m_cameraSize];
		m_captureImage[i] = new unsigned char[m_cameraSize];
		m_captureImage[i + m_frameNum] = new unsigned char[m_cameraSize];
	}

	m_systemInit = true;
}

/*
*
*/
bool CCameraDriver::drawLiveImage(string windowName, Mat inputImage)
{
	bool done = false;
	Mat colorImage = inputImage;
	if (inputImage.channels() < 3 && inputImage.elemSize() < 3)
	{
		demosaicing(inputImage, colorImage, COLOR_BayerBG2BGR);
	}
	namedWindow(windowName, WINDOW_NORMAL);
	resizeWindow(windowName, 800, 800 * inputImage.rows / inputImage.cols);
	imshow(windowName, colorImage);
	int c = waitKey(1);
	if (c == 27)
	{
		destroyAllWindows();
		done = true;
	}
	return done;
}

/*
*
*/
void CCameraDriver::livePreview(string windowName, float expTime)
{
	m_grab.setExposureTime(expTime);
	unsigned char* capturedImage = new unsigned char[m_cameraSize];
	while (1)
	{
		m_grab.captureSingleImageData(capturedImage, true);
		Mat previewImage = Mat(m_cameraRes, CV_8UC1, capturedImage);
		bool done = drawLiveImage(windowName, previewImage);
		if (done) break;
	}
	delete[] capturedImage;
}

/*
*
*/
int CCameraDriver::findBlackImageID()
{
	int darkID = 0;
	double darkMeanValue = 255 * 1000.0;

	float scaleX = 1.0f / (float)(m_cameraSize);
	for (int N = 0; N < m_frameNum; N++)
	{
		double meanValue = 0;
		for (int idx = 0; idx < m_cameraSize; idx++)
		{
			meanValue += (float)m_captureImage[N][idx] * scaleX;
		}
		if (meanValue < darkMeanValue)
		{
			darkMeanValue = meanValue;
			darkID = N;
		}
	}
	return darkID;
}

/*
*
*/
int CCameraDriver::grabImageSet(float expTime)
{
	m_grab.stopAcquisition();
	m_grab.setExposureTime(expTime);
	m_grab.startAcquisition();
	m_grab.captureImageSetData(m_captureImage, m_frameNum * 2, 0, true);
	m_grab.stopAcquisition();
	int blackID = findBlackImageID();

	return blackID;
}

/*
*
*/
bool CCameraDriver::saveRawImage(string folderDir)
{
	int ret = 0;
	for (int N = 0; N < m_frameNum; N++)
	{
		string fileName = folderDir + "/" + to_string(N) + ".png";
		Mat img = Mat(m_cameraRes, CV_8UC1, m_storeImage[N]);
		ret = imwrite(fileName, img);
	}
	return ret;
}

/*
*
*/
void CCameraDriver::grabPos(float expTime, string folderDir, bool isSaveRawImage)
{
	if (!m_systemInit)
	{
		cout << "system has not been initialized" << endl;
		return;
	}

	// turn on the camera based on camera serial number
	m_grab.openCamera(m_cameraSerialNo);

	// initialize camera
	m_grab.initCamera(m_cameraRes.width, m_cameraRes.height, m_frameRate, expTime, m_isHardwareTrigger);
	m_grab.startAcquisition();

	string windowName = "Check Image Quality (ESC to quit)";
	livePreview(windowName, expTime);

	// capture fringe images
	int blackID = grabImageSet(expTime);
	for (int k = 0; k < m_frameNum; k++)
	{
		memcpy(m_storeImage[k], m_captureImage[blackID + 2 + k], sizeof(m_captureImage[k][0]) * m_cameraSize);
	}
	// save fringe images if enabled
	if (isSaveRawImage)
	{
		saveRawImage(folderDir);
	}

	// turn off the camera
	m_grab.closeCamera();
}