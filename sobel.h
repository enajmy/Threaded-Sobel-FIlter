/********************************************************************
* File: sobel.h
*
* Description: header file for sobel.c (sobel filtering)
*
* Author: Ethan Najmy
*
* Revision History:
*	- 1.0: file created
*
********************************************************************/
#ifndef _SOBEL_H
#define _SOBEL_H
#include <string.h>
#include <pthread.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ximgproc.hpp>
#include <opencv2/imgcodecs.hpp>

// Variable Declarations (global)
#define THREADS 4
extern cv::Mat imageSections[THREADS];

// Initialize thread structures
struct threadArgs{
	int threadCount;
};

// Function Declaration
void *to442_sobelFilter(void *sectionCount);
void to442_threads(cv::Mat &frame);

#endif
