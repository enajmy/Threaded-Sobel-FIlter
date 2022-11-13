/********************************************************************
* File: sobel.c
*
* Description: this file implements a sobel filtering algorithm
*
* Author: Ethan Najmy
*
* Revisions:
*	- 1.0: file created
*
********************************************************************/

#include "sobel.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ximgproc.hpp>
#include <opencv2/imgcodecs.hpp>
/*--------------------------------------------------------------
* Function: to442_sobelFilter()
*
* Description: will convert an image/frame to grayscale
*
* void *sectionCount: takes the video frame Mat object as thread arg
* cv::Mat image: used to save thread arg to a local variable
* cv::Mat grayImage: used to create a 1 channel Mat object the same
* 	size as the input image
* blue, green, red, gray: stores the appropriate RBG/grayscale value
* cv::Mat sobelImage: used to create a Mat object that stores the
* 	sobel 
* 
* return: void (modifies parameters inputted rather than return)
*-------------------------------------------------------------*/

// Initialize global variable for imageSections
cv::Mat imageSections[THREADS];

void *to442_sobelFilter(void *sectionCount){	

	// Initialize thread arguments
	threadArgs *args = (threadArgs*)sectionCount;
	
	// Create a Mat objet for the input image and
	// set image to proper chunk of frame using threadCount arg
	cv::Mat image;
	imageSections[args->threadCount].copyTo(image);

	/* Create grayscale Mat object for 1 channel use
	 * Need to use a temp Mat to initialize same row/col
	 * as the input image for proper sizing */
	cv::Mat grayImage(image.rows, image.cols, CV_8UC1);

	// Itterate through columns and rows grabbing RGB values
	for (int i=0; i<image.rows; i++){
		for (int j=0; j<image.cols; j++){
			
			/* Grab RGB values:
			 * - Index [0] is blue
			 * - Index [1] is green
			 * - Index [2] is red
			 * OpenCV uses BGR rather than RGB */
			int blue = image.at<cv::Vec3b>(i,j)[0];
			int green = image.at<cv::Vec3b>(i,j)[1];
			int red = image.at<cv::Vec3b>(i,j)[2];
			
			// Apply ITU-R (BT.709) grayscale filtering
			double gray = (blue * 0.0722) 
				+ (green * 0.7152) + (red * 0.2126);
				
			// Save grayscale value to temporary Mat
			grayImage.at<uchar>(i,j) = gray;
		}	
	}
	/* Create temp Mat object for 1 channel use
	 * Need to use a temp Mat to initialize same row/col
	 * as the input image for proper sizing 
	 * (-2 on rows to remove the additional rows to prevent
	 * gaps in image while splicing) */
	cv::Mat sobelImage(grayImage.rows - 2, grayImage.cols, CV_8UC1);

	
	/* Itterate through all cols & rows (except border cols/rows
	* for sobel filtering */
	for (int i=1; i<(grayImage.rows - 1); i++){
		for (int j=1; j<(grayImage.cols - 1); j++){
		
			/* Calculate the x-component of Sobel using the
			 * following matrix:
			 * [-1 0 1
			 *  -2 0 2
			 *  -1 0 1] 
			 * Take absolute value as well */
			int x = abs((grayImage.at<uchar>(i-1,j-1) * -1)
				+ (grayImage.at<uchar>(i,j-1) * -2)
				+ (grayImage.at<uchar>(i+1,j-1) * -1)
				+ (grayImage.at<uchar>(i-1,j+1))
				+ (grayImage.at<uchar>(i,j+1) * 2)
				+ (grayImage.at<uchar>(i+1,j+1)));
				
			// Ensure values are within grayscale range (0-255)
			if (x < 0)
				x = 0;
			else if (x > 255)
				x = 255;
				
			/* Calculate the y-component of Sobel using the
			 * following matrix:
			 * [ 1  2  1
			 *   0  0  0
			 *  -1 -2 -1 ] 
			 * Take absolute value as well */
			int y = abs((grayImage.at<uchar>(i-1,j-1))
				+ (grayImage.at<uchar>(i-1,j) * 2)
				+ (grayImage.at<uchar>(i-1,j+1))
				+ (grayImage.at<uchar>(i+1,j-1) * -1)
				+ (grayImage.at<uchar>(i+1,j) * -2)
				+ (grayImage.at<uchar>(i+1,j+1) * -1));
				
			// Ensure values are within grayscale range (0-255)
			if (y < 0)
				y = 0;
			else if (y > 255)
				y = 255;
				
			// Add x & y components and save to sobel Mat
			// -1 on i (rows) because of additional pixels
			// to prevent black lines when splicing images together
			sobelImage.at<uchar>(i-1,j) = (x+y);
		}	
	}

	// Save the sobel image to its respective section
	imageSections[args->threadCount] = sobelImage;

	pthread_exit(NULL);
}
/*--------------------------------------------------------------
* Function: to442_threads()
*
* Description: used to run and process the threads for image filtering
*
* return: void
* 
* cv::Mat outImage: used to store the output image
* width: used to make calling frame width easier
* height: used to make calling frame height easier
* y[4]: used to store image chunk values in 4ths for iteration
*-------------------------------------------------------------*/
void to442_threads(cv::Mat &frame){

	// Create output Mat object (1 channel)
	cv::Mat outImage(frame.rows, frame.cols, CV_8UC1);
	
	// Initalize threads and arguments, make joinable
	pthread_t thread[THREADS];
	struct threadArgs args[THREADS];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	// Initialize width and frame and image chunk sizes
	int width = frame.cols;
	int height = (frame.rows/4);
	int y[4] = {0, frame.rows/4, frame.rows/2, (frame.rows*3)/4};
	
	
	// Split images into respective chunks (vertically)
	imageSections[0] = frame(cv::Rect(0, 0, width, height + 2));
	imageSections[1] = frame(cv::Rect(0, frame.rows/4 - 2, width, height + 2));
	imageSections[2] = frame(cv::Rect(0, frame.rows/2 - 2, width, height + 2));
	imageSections[3] = frame(cv::Rect(0, (frame.rows*3)/4 - 2, width, height + 2));
	
	// Start threads & check for errors
	for (int i = 0; i < THREADS; i++){
		args[i].threadCount = i;
		int err = pthread_create(&thread[i], NULL, to442_sobelFilter, (void *)&args[i]);
		if (err)
			printf("Thread creation failed! Thread #: %d\n", i);
	}
		
	// When thread completes, join & check for erros
	for (int i = 0; i < THREADS; i++){
		int err = pthread_join(thread[i], NULL);
		if (err)
			printf("Thread join failed! Thread #: %d\n", i);
	}
	
	// Copy the image sections to the output Mat
	for (int i = 0; i < THREADS; i++){
		imageSections[i].copyTo(outImage(cv::Rect(0, y[i], width, height)));
	}

	// Show the image/frame that has been filtered
	imshow("Sobel Filter", outImage);
	
	// Delay
	cv::waitKey(1);
}
