//
//  cvImageUtil.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 15..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef cvImageUtil_h
#define cvImageUtil_h

#include <cocos2d.h>
#include "use_opencv.h"


class cvImageUtil
{
public:
    static cv::Mat createCvMatFromRaw(unsigned char *rawData, int rawXW, int rawYW, int ch)
    {
        cv::Mat cvMat( rawYW, rawXW, CV_8UC4); // 8 bits per component, 4 channels
        
        for (int py=0; py<rawYW; py++) {
            for (int px=0; px<rawXW; px++) {
                int nBasePos = ((rawXW * py)+px) * ch;
                cvMat.at<cv::Vec4b>(py, px) = cv::Vec4b(rawData[nBasePos + 0],  // 赤
                                                        rawData[nBasePos + 1],  // 緑
                                                        rawData[nBasePos + 2],  // 青
                                                        0xFF);   // Alafa
                
            }
        }
        
        return cvMat;
    }
    
    static cv::Mat ccImage2cvMat(cocos2d::Image* ccImage)
    {
        int imageSize = (int)ccImage->getDataLen();
        
        int imageXW = ccImage->getWidth();
        int imageYW = ccImage->getHeight();
        unsigned char * srcData = ccImage->getData();
        
        int ch = imageSize/(imageXW*imageYW);
        
        return createCvMatFromRaw(srcData, imageXW, imageYW, ch);
    }
    
    static unsigned char * getMatData(cv::Mat cvMat, int *len)
    {
        int elemSize = (int)cvMat.elemSize();
        unsigned char *srcData;
        
        *len = (int)(elemSize * cvMat.total());
        
        int width  = cvMat.cols;
        int height = cvMat.rows;
        
        
        unsigned char * pTempData = NULL;
        if(1==elemSize){
            int bytesPerComponent = 4;
            long size = bytesPerComponent * width * height;
            pTempData = static_cast<unsigned char*>(malloc(size * sizeof(unsigned char)));
            
            
            if(pTempData){
                int imageYW=height;
                int imageXW=width;
                
                unsigned char * matData = (unsigned char *)cvMat.data;
                
                for (int py=0; py<imageYW; py++) {
                    for (int px=0; px<imageXW; px++) {
                        int matPixPos = (imageXW * py * 1) + (px * 1);
                        int pixPos = (imageXW * py * bytesPerComponent) + (px * bytesPerComponent);
                        pTempData[pixPos+0]=matData[matPixPos];    // Red
                        pTempData[pixPos+1]=matData[matPixPos];    // Green
                        pTempData[pixPos+2]=matData[matPixPos];    // Blue
                        pTempData[pixPos+3]=0xFF;    // Alpha
                    }
                }
            }
            
            srcData = pTempData;
        } else {
            srcData = (unsigned char *)cvMat.data;
        }
        
        if (pTempData) {
            free(pTempData);
        }
        
        return srcData;
    }
    
    static cocos2d::Image* cvMat2ccImage(cv::Mat cvMat)
    {
        int elemSize = (int)cvMat.elemSize();
        unsigned char *srcData;
        
        
        int width  = cvMat.cols;
        int height = cvMat.rows;
        
        
        unsigned char * pTempData = NULL;
        if(1==elemSize){
            int bytesPerComponent = 4;
            long size = bytesPerComponent * width * height;
            pTempData = static_cast<unsigned char*>(malloc(size * sizeof(unsigned char)));
            
            
            if(pTempData){
                int imageYW=height;
                int imageXW=width;
                
                unsigned char * matData = (unsigned char *)cvMat.data;
                
                for (int py=0; py<imageYW; py++) {
                    for (int px=0; px<imageXW; px++) {
                        int matPixPos = (imageXW * py * 1) + (px * 1);
                        int pixPos = (imageXW * py * bytesPerComponent) + (px * bytesPerComponent);
                        pTempData[pixPos+0]=matData[matPixPos];    // Red
                        pTempData[pixPos+1]=matData[matPixPos];    // Green
                        pTempData[pixPos+2]=matData[matPixPos];    // Blue
                        pTempData[pixPos+3]=0xFF;    // Alpha
                    }
                }
            }
            
            srcData = pTempData;
        } else {
            srcData = (unsigned char *)cvMat.data;
        }
        long size = 4  * width * height;
        
        cocos2d::Image* ccImage = new cocos2d::Image();
        
        if (ccImage && ccImage->initWithRawData( srcData,
                                                size,
                                                width,
                                                height,
                                                elemSize * 8
                                                )
            ) {
            if(pTempData){
                free(pTempData);
            }
            
            
            return ccImage;
        }
        return NULL;
    }
    
};

#endif /* cvImageUtil_h */
