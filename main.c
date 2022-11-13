/********************************************************************
* File: main.c
*
* Description: main file for sobel filter algorithm
*
* Author: Ethan Najmy
*
* Revision History:
*	- 1.0: file created
*
********************************************************************/
#include "sobel.h"
/*--------------------------------------------------------------
* Function: main()
*
* Description: main function, runs program
*
* cv::Mat frame: used to store individual video frames
*
* return: int
*-------------------------------------------------------------*/
int main(void){
	// Variable declarations
	cv::Mat frame;
	
	// Use OpenCV to read a video file
	cv::VideoCapture capture("motocross.mp4");
	
	// While loop to read all frames
	while(1){
		
		// Save individual frame (starting at first frame)
		capture.read(frame);
		
		// If frame is empty (video is over), break and end
		if (frame.empty())
			break;
		
		// Call sobel filtering functions w/threads
		to442_threads(frame);
	}
	return 0;
}
