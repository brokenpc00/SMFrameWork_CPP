//
//  DownloadTaskApple.m
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 17..
//
//
#include "DownloadTask.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "ImageDownloader.h"
#include <Photos/PHAsset.h>
#include <Photos/PHImageManager.h>
#include <opencv2/imgproc.hpp>
#include "CCImageEx.h"
//#import <ImageIO/ImageIO.h>
//#include <external/webp/include/ios/encode.h>
//#include <stdio.h>
//#include <sys/time.h>

#define __CHECK_THREAD_INTERRUPTED__  { std::this_thread::sleep_for(std::chrono::milliseconds(1)); if (!_running) break; }

#define MAX_WIDTH       (360)

std::string DownloadTask::getPHAssetIdentifier() {
    if (_phAssetObject) {
        auto asset = static_cast<PHAsset*>(_phAssetObject);
        auto identifier = std::string([asset.localIdentifier UTF8String]);
        
        return identifier;
    }
    return "";
}

void DownloadTask::procLoadFromLocalIdentifierThumbnailThread() {
    
    if (@available(iOS 13, *)) {
        // _phAssetObject에서 pixel data를 뽑아서 resample한다.
        
        _uiImageObject = nullptr;
        
        do {
            PHImageRequestOptions* opts = [PHImageRequestOptions new];
            opts.resizeMode = PHImageRequestOptionsResizeModeFast;
            opts.synchronous = YES;
            
            auto asset = static_cast<PHAsset*>(_phAssetObject);
            
            [[PHImageManager defaultManager] requestImageForAsset: asset
                                                       targetSize: CGSizeMake(300, 300)
                                                      contentMode: PHImageContentModeAspectFit
                                                          options: opts
                                                    resultHandler: ^(UIImage* result, NSDictionary* info) {
                                                        if (result) {
                                                            @autoreleasepool {
    uint8_t* assetData = nullptr;
    int assetDataWidth = 0;
    int assetDataHeight = 0;

                                                                CGImageRef imageRef = result.CGImage;
                                                                CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
                                                                
                                                                assetDataWidth = (int)CGImageGetWidth(imageRef);
                                                                assetDataHeight = (int)CGImageGetHeight(imageRef);
                                                                assetData = (uint8_t*)malloc(assetDataWidth * assetDataHeight * 4);
                                                                
                                                                CGContextRef conextRef = CGBitmapContextCreate(assetData,
                                                                                                               (size_t)assetDataWidth,
                                                                                                               (size_t)assetDataHeight,
                                                                                                               8, // bit per pixel : 항상 8로 들어옴
                                                                                                               (size_t)assetDataWidth * 4,
                                                                                                               colorSpace,
                                                                                                               kCGImageAlphaPremultipliedLast);
    
                                                                CGContextDrawImage(conextRef, CGRectMake(0, 0, assetDataWidth, assetDataHeight), imageRef);
    
                                                                if (colorSpace) {
                                                                    CGColorSpaceRelease(colorSpace);
                                                                    colorSpace = nil;
                                                                }
                                                                
                                                                if (conextRef) {
                                                                    CGContextRelease(conextRef);
                                                                    conextRef = nil;
                                                                }
                                                                
                                                                imageRef = nil;
                                                                
                                                                if (assetData == nullptr || assetDataWidth <= 0 || assetDataHeight <= 0)
                                                                {
                                                                    CC_SAFE_FREE(assetData);
                                                                    return;
                                                                }
                                                                
                                                                if (_config._resamplePolicy == DownloadConfig::ResamplePolicy::NONE) {
                                                                    auto image = new CCImageEx();
                                                                    if (image->initWithRawDataEx(assetData, assetDataWidth*assetDataHeight*4, assetDataWidth, assetDataHeight, 32, true)) {
                                                                        _imageEntry = ImageCacheEntry::createEntry(image, nullptr);
                                                                    } else {
                                                                        CC_SAFE_RELEASE(image);
                                                                    }
                                                                    CC_SAFE_FREE(assetData);
                                                                } else {
                                                                    // crop square
                                                                    cv::Mat src(assetDataHeight, assetDataWidth, CV_8UC4, assetData);
                                                                    cv::Mat dst;
                                                                    cv::Rect rect;
                                                                    int sideLength;
                                                                    
                                                                    if (assetDataHeight > assetDataWidth) {
                                                                        // 세로가 길다
                                                                        int offset = (assetDataHeight - assetDataWidth) / 2;
                                                                        rect = cv::Rect(0, offset, assetDataWidth, assetDataWidth);
                                                                        sideLength = assetDataWidth;
                                                                    } else {
                                                                        // 가로가 길다
                                                                        int offset = (assetDataWidth - assetDataHeight) / 2;
                                                                        rect = cv::Rect(offset, 0, assetDataHeight, assetDataHeight);
                                                                        sideLength = assetDataHeight;
                                                                    }
                                                                    
                                                                    // 크롭
                                                                    if (sideLength > MAX_WIDTH) {
                                                                        // 리사이즈
                                                                        sideLength = MAX_WIDTH;
                                                                        cv::resize(src(rect), dst, cv::Size(MAX_WIDTH, MAX_WIDTH), 0, 0, CV_INTER_LINEAR);
                                                                        CC_SAFE_FREE(assetData);
                                                                    } else {
                                                                        src(rect).copyTo(dst);
                                                                        CC_SAFE_FREE(assetData);
                                                                    }
                                                                    
                                                                    if (dst.data != nullptr && dst.cols > 0 && dst.rows > 0) {
                                                                        cv::cvtColor(dst, dst, CV_BGRA2BGR);
                                                                        
                                                                        auto image = new CCImageEx();
                                                                        if (image->initWithRawDataEx(dst.data, sideLength*sideLength*4, sideLength, sideLength, 24, true)) {
                                                                            _imageEntry = ImageCacheEntry::createEntry(image, nullptr);
                                                                        } else {
                                                                            CC_SAFE_RELEASE(image);
                                                                        }
                                                                    }
                                                                    
                                                                    src.release();
                                                                    dst.release();
                                                                }
                                                                
                                                                CC_SAFE_FREE(assetData);
                                                                                                                            
                                                                if (_imageEntry.get() == nullptr) {
                                                                    return;
                                                                }
                                                                
                                                                // 성공 통보
                                                                _downloader->handleState(shared_from_this(), ImageDownloader::State::DECODE_SUCCESS);

                                                                return;
                                                            }
                                                        }
                                                    }];
            
            [opts release];
            return;
        } while (0);

        // 여기는 실패한 경우
        
        //------------------- BREAK on INTERRUPT or ERROR---------------------
        
        _imageEntry.reset();
        _cacheEntry.reset();
        
        _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_FAILED);
    } else {
        uint8_t* assetData = nullptr;
        int assetDataWidth = 0;
        int assetDataHeight = 0;
        _uiImageObject = nullptr;

    do {
        @autoreleasepool {
            auto asset = static_cast<PHAsset*>(_phAssetObject);

            PHImageRequestOptions* opts = [PHImageRequestOptions new];
            opts.resizeMode = PHImageRequestOptionsResizeModeFast;
            opts.synchronous = YES;

            [[PHImageManager defaultManager] requestImageForAsset: asset
                                                       targetSize: CGSizeMake(300, 300)
                                                      contentMode: PHImageContentModeAspectFit
                                                          options: opts
                                                    resultHandler: ^(UIImage* result, NSDictionary* info) {
                                                        if (result) {
                                                            _uiImageObject = (void*)result;
                                                        }
                                                    }];

            [opts release];

            __CHECK_THREAD_INTERRUPTED__;

            auto uiImage = static_cast<UIImage*>(_uiImageObject);
            _uiImageObject = nullptr;

            if (uiImage == nullptr)
                    break;

            CGImageRef imageRef = uiImage.CGImage;
            CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

            assetDataWidth = (int)CGImageGetWidth(imageRef);
            assetDataHeight = (int)CGImageGetHeight(imageRef);
            assetData = (uint8_t*)malloc(assetDataWidth * assetDataHeight * 4);

            CGContextRef conextRef = CGBitmapContextCreate(assetData,
                                                           (size_t)assetDataWidth,
                                                           (size_t)assetDataHeight,
                                                           8, // bit per pixel : 항상 8로 들어옴
                                                           (size_t)assetDataWidth * 4,
                                                           colorSpace,
                                                           kCGImageAlphaPremultipliedLast);

            CGContextDrawImage(conextRef, CGRectMake(0, 0, assetDataWidth, assetDataHeight), imageRef);

            if (colorSpace) {
                CGColorSpaceRelease(colorSpace);
                colorSpace = nil;
                }

            if (conextRef) {
                CGContextRelease(conextRef);
                conextRef = nil;
            }

            uiImage = nullptr;
            imageRef = nullptr;
        }

        if (assetData == nullptr || assetDataWidth <= 0 || assetDataHeight <= 0)
                    break;

                __CHECK_THREAD_INTERRUPTED__;

        if (_config._resamplePolicy == DownloadConfig::ResamplePolicy::NONE) {
            auto image = new CCImageEx();
            if (image->initWithRawDataEx(assetData, assetDataWidth*assetDataHeight*4, assetDataWidth, assetDataHeight, 32, true)) {
                _imageEntry = ImageCacheEntry::createEntry(image, nullptr);
            } else {
                CC_SAFE_RELEASE(image);
                }
            CC_SAFE_FREE(assetData);
        } else {
            // crop square
            cv::Mat src(assetDataHeight, assetDataWidth, CV_8UC4, assetData);
            cv::Mat dst;
            cv::Rect rect;
            int sideLength;

            if (assetDataHeight > assetDataWidth) {
                // 세로가 길다
                int offset = (assetDataHeight - assetDataWidth) / 2;
                rect = cv::Rect(0, offset, assetDataWidth, assetDataWidth);
                sideLength = assetDataWidth;
            } else {
                // 가로가 길다
                int offset = (assetDataWidth - assetDataHeight) / 2;
                rect = cv::Rect(offset, 0, assetDataHeight, assetDataHeight);
                sideLength = assetDataHeight;
            }

            // 크롭
            if (sideLength > MAX_WIDTH) {
                // 리사이즈
                sideLength = MAX_WIDTH;
                cv::resize(src(rect), dst, cv::Size(MAX_WIDTH, MAX_WIDTH), 0, 0, CV_INTER_LINEAR);
                CC_SAFE_FREE(assetData);
            } else {
                src(rect).copyTo(dst);
                CC_SAFE_FREE(assetData);
            }

            if (dst.data != nullptr && dst.cols > 0 && dst.rows > 0) {
                cv::cvtColor(dst, dst, CV_BGRA2BGR);

                auto image = new CCImageEx();
                if (image->initWithRawDataEx(dst.data, sideLength*sideLength*4, sideLength, sideLength, 24, true)) {
                    _imageEntry = ImageCacheEntry::createEntry(image, nullptr);
                } else {
                    CC_SAFE_RELEASE(image);
                }
            }

            src.release();
            dst.release();
        }

        CC_SAFE_FREE(assetData);


        __CHECK_THREAD_INTERRUPTED__;
        if (_imageEntry.get() == nullptr) {
            break;
        }

        // 성공 통보
        _downloader->handleState(shared_from_this(), ImageDownloader::State::DECODE_SUCCESS);
        // 바로 리턴
        return;

    } while (0);

    CC_SAFE_FREE(assetData);

    //------------------- BREAK on INTERRUPT or ERROR---------------------

    _imageEntry.reset();
    _cacheEntry.reset();

    _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_FAILED);
    }

}


void DownloadTask::procLoadFromLocalIdentifierThread() {
    
    if (@available(iOS 13, *)) {
        _uiImageObject = nullptr;
        
        do {
            auto asset = static_cast<PHAsset*>(_phAssetObject);
            
            PHImageRequestOptions* opts = [PHImageRequestOptions new];
            opts.resizeMode = PHImageRequestOptionsResizeModeFast;
            opts.synchronous = YES;
            
            [[PHImageManager defaultManager] requestImageForAsset: asset
                                                       targetSize: CGSizeMake(_config._maxSideLength, _config._maxSideLength)
                                                      contentMode: PHImageContentModeAspectFit
                                                          options: opts
                                                    resultHandler: ^(UIImage* result, NSDictionary* info) {
                                                        if (result) {
                                                            @autoreleasepool {
                                                                uint8_t* assetData = nullptr;
                                                                int assetDataWidth = 0;
                                                                int assetDataHeight = 0;
                                                                
                                                                CGImageRef imageRef = result.CGImage;
                                                                CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
                                                                
                                                                assetDataWidth = (int)CGImageGetWidth(imageRef);
                                                                assetDataHeight = (int)CGImageGetHeight(imageRef);
                                                                assetData = (uint8_t*)malloc(assetDataWidth * assetDataHeight * 4);
                                                                
                                                                CGContextRef conextRef = CGBitmapContextCreate(assetData,
                                                                                                               (size_t)assetDataWidth,
                                                                                                               (size_t)assetDataHeight,
                                                                                                               8, // bit per pixel : 항상 8로 들어옴
                                                                                                               (size_t)assetDataWidth * 4,
                                                                                                               colorSpace,
                                                                                                               kCGImageAlphaPremultipliedLast);
                                                                
                                                                CGContextDrawImage(conextRef, CGRectMake(0, 0, assetDataWidth, assetDataHeight), imageRef);
                                                                
                                                                if (colorSpace) {
                                                                    CGColorSpaceRelease(colorSpace);
                                                                    colorSpace = nil;
                                                                }
                                                                
                                                                if (conextRef) {
                                                                    CGContextRelease(conextRef);
                                                                    conextRef = nil;
                                                                }
                                                                
                                                                imageRef = nil;
                                                                
                                                                if (assetData == nullptr || assetDataWidth <= 0 || assetDataHeight <= 0)
                                                                    return;
                                                                
                                                                auto image = new CCImageEx();
                                                                if (image->initWithRawDataEx(assetData, assetDataWidth*assetDataHeight*4, assetDataWidth, assetDataHeight, 32, true)) {
                                                                    _imageEntry = ImageCacheEntry::createEntry(image, nullptr);
                                                                } else {
                                                                    CC_SAFE_RELEASE(image);
                                                                }
                                                                CC_SAFE_FREE(assetData);
                                                                
                                                                if (_imageEntry.get() == nullptr) {
                                                                    CC_SAFE_FREE(assetData);
                                                                    return;
                                                                }
                                                                
                                                                // 성공 통보
                                                                _downloader->handleState(shared_from_this(), ImageDownloader::State::DECODE_SUCCESS);
                                                                // 바로 리턴
                                                                return;
                                                            }
                                                        }
                                                        
                                                    }];
            [opts release];
            return;
        } while (0);
        
        _imageEntry.reset();
        _cacheEntry.reset();
        
        _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_FAILED);

    } else {
    uint8_t* assetData = nullptr;
    int assetDataWidth = 0;
    int assetDataHeight = 0;
    _uiImageObject = nullptr;
    
    do {
        @autoreleasepool {
            auto asset = static_cast<PHAsset*>(_phAssetObject);
        
            PHImageRequestOptions* opts = [PHImageRequestOptions new];
            opts.resizeMode = PHImageRequestOptionsResizeModeFast;
            opts.synchronous = YES;
        
            [[PHImageManager defaultManager] requestImageForAsset: asset
                                                       targetSize: CGSizeMake(_config._maxSideLength, _config._maxSideLength)
                                                      contentMode: PHImageContentModeAspectFit
                                                          options: opts
                                                    resultHandler: ^(UIImage* result, NSDictionary* info) {
                                                        if (result) {
                                                            _uiImageObject = (void*)result;
                                                        }
        
                                                    }];
        
            [opts release];
        
                __CHECK_THREAD_INTERRUPTED__;
        
            auto uiImage = static_cast<UIImage*>(_uiImageObject);
            _uiImageObject = nullptr;
            
            if (uiImage == nullptr)
                break;
                
            CGImageRef imageRef = uiImage.CGImage;
            CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
                
            assetDataWidth = (int)CGImageGetWidth(imageRef);
            assetDataHeight = (int)CGImageGetHeight(imageRef);
            assetData = (uint8_t*)malloc(assetDataWidth * assetDataHeight * 4);
                
            CGContextRef conextRef = CGBitmapContextCreate(assetData,
                                                           (size_t)assetDataWidth,
                                                           (size_t)assetDataHeight,
                                                           8, // bit per pixel : 항상 8로 들어옴
                                                           (size_t)assetDataWidth * 4,
                                                  colorSpace,
                                                  kCGImageAlphaPremultipliedLast);
                
            CGContextDrawImage(conextRef, CGRectMake(0, 0, assetDataWidth, assetDataHeight), imageRef);
                
            if (colorSpace) {
                CGColorSpaceRelease(colorSpace);
                colorSpace = nil;
        }
        
            if (conextRef) {
                CGContextRelease(conextRef);
                conextRef = nil;
            }
            
            uiImage = nullptr;
            imageRef = nullptr;
            }
            
        if (assetData == nullptr || assetDataWidth <= 0 || assetDataHeight <= 0)
                break;
            
        __CHECK_THREAD_INTERRUPTED__;
            
        //--------------------------------------------------------------------
        // TODO : ResamplePolicy 처리해야 한다.
        //--------------------------------------------------------------------
        // ... 여기에 ...
        //--------------------------------------------------------------------
            
        auto image = new CCImageEx();
        if (image->initWithRawDataEx(assetData, assetDataWidth*assetDataHeight*4, assetDataWidth, assetDataHeight, 32, true)) {
                _imageEntry = ImageCacheEntry::createEntry(image, nullptr);
            } else {
                CC_SAFE_RELEASE(image);
            }
        CC_SAFE_FREE(assetData);
        
        __CHECK_THREAD_INTERRUPTED__;
        if (_imageEntry.get() == nullptr) {
            break;
        }
        
        // 성공 통보
        _downloader->handleState(shared_from_this(), ImageDownloader::State::DECODE_SUCCESS);
        // 바로 리턴
        return;
        
    } while (0);
    
    CC_SAFE_FREE(assetData);
    
    //------------------- BREAK on INTERRUPT or ERROR---------------------
    
    _imageEntry.reset();
    _cacheEntry.reset();
    
    _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_FAILED);
    }
    

}
#endif
