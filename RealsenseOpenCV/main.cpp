//
//  main.cpp
//  RealsenseOpenCV
//
//  Created by Ying Gaoxuan on 16/7/14.
//  Copyright © 2016年 Ying Gaoxuan. All rights reserved.
//

#include <cstdio>
#include <iostream>
#include "rs.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <unistd.h>

#define TARGET_DEPTH_MIN 1000
#define TARGET_DEPTH_MAX 10000

#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480

using namespace std;
using namespace cv;

//
// main process
//
int main(int argc, char* argv[]) try {
    
    // Create a context object. This object owns the handles to all connected realsense devices.
    rs::context ctx;
    printf("There are %d connected RealSense devices.\n", ctx.get_device_count());
    if(ctx.get_device_count() == 0) return EXIT_FAILURE;
    
    rs::device * dev = ctx.get_device(0);
    
    // Configure depth to run at VGA resolution at 30 frames per second
    //dev->enable_stream(rs::stream::depth, IMAGE_WIDTH, IMAGE_HEIGHT, rs::format::z16, 30);
    dev->enable_stream(rs::stream::depth, rs::preset::best_quality);
    dev->enable_stream(rs::stream::color, rs::preset::best_quality);
    dev->start();
    
    // Determine depth value corresponding to one meter
    const uint16_t one_meter = static_cast<uint16_t>(1.0f / dev->get_depth_scale());
    
    // Opencv values
    IplImage* depth_img = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, 1);
    cvNamedWindow("realsense_depth", CV_WINDOW_AUTOSIZE);
    
    IplImage* color_img = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, 3);
    cvNamedWindow("realsense_color", CV_WINDOW_AUTOSIZE);
    
    
    while(true)
    {
        
        dev->wait_for_frames();
        
        const uint16_t * depth_frame = reinterpret_cast<const uint16_t *>(dev->get_frame_data(rs::stream::depth_aligned_to_color));
        
        for(int y=0; y<480; ++y)
        {
            for(int x=0; x<640; ++x)
            {
                int depth = *depth_frame++;
                char* output_pixel = &(depth_img->imageData[(y*depth_img->widthStep) + x]);
                
                if ( TARGET_DEPTH_MIN < depth && depth < TARGET_DEPTH_MAX)
                {
                    float temp = (float)depth / 10000.0;
                    *output_pixel = (int)(temp*255.0);
                }
                else
                {
                    *output_pixel = (char)255;
                }
                
            }
        }
        
      
        
        const uint8_t * color_frame = reinterpret_cast<const uint8_t *>(dev->get_frame_data(rs::stream::color));
        
        for(int y=0; y<480; ++y)
        {
            for(int x=0; x<640; ++x)
            {
                for(int k=2; k>=0; --k)
                {
                    int color = *color_frame++;
                    char* color_output_pixel = &(color_img->imageData[(y*color_img->widthStep) + x*3 +k]);
                    *color_output_pixel = color;
                }
            }
        }
        
        cvShowImage("realsense_depth", depth_img);
        cvShowImage("realsense_color", color_img);
        cvWaitKey(10);
        
    }
    
    return EXIT_SUCCESS;
} catch(const rs::error & e) {
    // Method calls against librealsense objects may throw exceptions of type rs::error
    printf("rs::error was thrown when calling %s(%s):\n", e.get_failed_function().c_str(), e.get_failed_args().c_str());
    printf("    %s\n", e.what());
    return EXIT_FAILURE;
}