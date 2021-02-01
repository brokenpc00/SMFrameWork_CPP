//
//  AppleCamera.m
//  iPet
//
//  Created by KimSteve on 2017. 7. 5..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#import "AppleCamera.h"
#import <AVFoundation/AVFoundation.h>
#import <AVFoundation/AVCaptureDevice.h> // For access to the camera
#import <AVFoundation/AVCaptureOutput.h> // For capturing frames
#import <CoreVideo/CVPixelBuffer.h> // for using pixel format types
#include <queue>
#include <platform/CCImage.h>
#include <renderer/CCTexture2D.h>

@interface AppleCamera()<AVCaptureVideoDataOutputSampleBufferDelegate, AVCaptureMetadataOutputObjectsDelegate>
{
    AVCaptureDevice* _cameraDevice; // A pointer to the front or to the back camera
    
    AVCaptureSession* _captureSession; // Lets us set up and control the camera
    
    AVCaptureDeviceInput* _cameraInput; // This is the data input for the camera that allows u
    
    AVCaptureVideoDataOutput* _videoOutput; // For the video frame data from the camera
    
    AVCaptureStillImageOutput* _stillImageOutput;
    
    dispatch_queue_t _sessionQueue;
    
    int _cameraId;
    
    int _frameReceived;
    
    bool _stopCameraAfterCapture;
    
    std::shared_ptr<CameraPlayer> _cameraPlayer;
    
    // video frame queue
    NSLock* _queueLock;
    
    std::queue<VideoFrame*> _idleQueue;
    
    std::queue<VideoFrame*> _decodeQueue;
    bool bCodeScanner;
    
    float frameWidth;
    float frameHeight;
    float cropHalfWidth;
    float cropHalfHeight;
}
@end


@implementation AppleCamera
- ( id ) initWithHost : (std::shared_ptr<CameraPlayer>) player withCodeScanner:(BOOL)codeScanner
{
    if (self = [super init]) {
        frameWidth = -1;
        frameHeight = -1;
        bCodeScanner = codeScanner;
        _captureSession    = NULL;
        _cameraDevice      = NULL;
        _cameraInput       = NULL;
        _videoOutput       = NULL;
        _stillImageOutput  = NULL;
        _frameReceived = 0;
        _cameraId = CameraDeviceConst::Facing::BACK;
        _cameraPlayer = player;
        
        _sessionQueue = dispatch_queue_create( "camera session queue", DISPATCH_QUEUE_SERIAL );
        
        // TODO : 테스트 필요 (더블 or 트리플)
        // ensure video frame buffer
        _queueLock = [ [ NSLock alloc ] init ];
        [ self queueIdleFrame:new VideoFrame() ]; // 1st buffer
        [ self queueIdleFrame:new VideoFrame() ]; // 2nd buffer
    }
    
    return self;
}

- (void) dealloc
{
    // ARC Not use this...
    //    dispatch_release(_sessionQueue);
    
    // release buffers
    while (auto frame = [ self dequeueIdleFrame ]) {
        if (frame->buffer) {
            free(frame->buffer);
        }
        delete frame;
    }
    
    while (auto frame = [ self dequeueDecodeFrame ]) {
        if (frame->buffer) {
            free(frame->buffer);
        }
        delete frame;
    }
    
    _cameraPlayer.reset();
}

- ( BOOL ) attachCameraToCaptureSession
{
    assert( NULL != _cameraDevice );
    assert( NULL != _captureSession );
    
    
    // 1. Initialize the camera input
    _cameraInput = NULL;
    
    // 2. Request a camera input from the camera
    NSError* error = NULL;
    _cameraInput = [ AVCaptureDeviceInput deviceInputWithDevice: _cameraDevice error: &error ];
    
    // 2.1. Check if we've got any errors
    if ( NULL != error )
    {
        // TODO: send an error event to ActionScript
        return false;
    }
    
    // 3. We've got the input from the camera, now attach it to the capture session:
    if ( [ _captureSession canAddInput: _cameraInput ] )
    {
        [ _captureSession addInput: _cameraInput ];
    }
    else
    {
        // TODO: send an error event to ActionScript
        return false;
    }
    
    // 4. Done, the attaching was successful, return true to signal that
    return true;
}

- ( BOOL ) findCamera: ( int ) cameraId
{
    // 0. Make sure we initialize our camera pointer:
    
    #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    _cameraDevice = NULL;
    // 1. Get a list of available devices:
    // specifying AVMediaTypeVideo will ensure we only get a list of cameras, no microphones
    NSArray* devices = [ AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo ];
    
    // 2. Iterate through the device array and if a device is a camera, check if it's the one we want:
    for ( AVCaptureDevice* device in devices )
    {
        if ( (cameraId == CameraDeviceConst::Facing::FRONT) && AVCaptureDevicePositionFront == [ device position ] )
        {
            // We asked for the front camera and got the front camera, now keep a pointer to it:
            _cameraDevice = device;
            _cameraId = CameraDeviceConst::Facing::FRONT;
        }
        else if ( (cameraId == CameraDeviceConst::Facing::BACK) && AVCaptureDevicePositionBack == [ device position ] )
        {
            // We asked for the back camera and here it is:
            _cameraDevice = device;
            _cameraId = CameraDeviceConst::Facing::BACK;
        }
    }
    #else
    _cameraDevice = [ AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo ][0];
    _cameraId = CameraDeviceConst::Facing::FRONT;
    #endif
    
    // 3. Set a frame rate for the camera:
    if ( NULL != _cameraDevice )
    {
        // We firt need to lock the camera, so noone else can mess with its configuration:
        if ( [ _cameraDevice lockForConfiguration: nil ] )
        {
            /*
             // Set a minimum frame rate of 10 frames per second
             [ _cameraDevice setActiveVideoMinFrameDuration: CMTimeMake( 10, 100 ) ];
             
             // and a maximum of 30 frames per second
             [ _cameraDevice setActiveVideoMaxFrameDuration: CMTimeMake( 10, 300 ) ];
             */
            
            float frameRate = 30;
            for (AVCaptureDeviceFormat* vFormat in [ _cameraDevice formats ]) {
                CMFormatDescriptionRef description= vFormat.formatDescription;
                float maxRate = ((AVFrameRateRange*) [vFormat.videoSupportedFrameRateRanges objectAtIndex:0]).maxFrameRate;
                
                if (maxRate >= frameRate && CMFormatDescriptionGetMediaSubType(description) == kCVPixelFormatType_32BGRA) {
                    if ([ _cameraDevice lockForConfiguration:nil ]) {
                        _cameraDevice.activeFormat = vFormat;
                        [ _cameraDevice setActiveVideoMinFrameDuration:CMTimeMake(10, frameRate * 10) ];
                        [ _cameraDevice setActiveVideoMaxFrameDuration:CMTimeMake(10, frameRate * 10) ];
                        [ _cameraDevice unlockForConfiguration];
                        break;
                    }
                }
            }
            
            
            [ _cameraDevice unlockForConfiguration ];
        }
    } else {
        _cameraId = CameraDeviceConst::Facing::UNSPECIFIED;
    }
    
    // 4. If we've found the camera we want, return true
    return ( NULL != _cameraDevice );
}

- ( void ) setupVideoOutput
{
    // 1. Create the video data output
    _videoOutput = [ [ AVCaptureVideoDataOutput alloc ] init ];
    
    // 2. Create a queue for capturing video frames
    dispatch_queue_t captureQueue = dispatch_queue_create( "camera capture queue", NULL );//DISPATCH_QUEUE_SERIAL );
    
    // 3. Use the AVCaptureVideoDataOutputSampleBufferDelegate capabilities of CameraDelegate:
    [ _videoOutput setSampleBufferDelegate: self queue: captureQueue ];
    
    // 4. Set up the video output
    // 4.1. Do we care about missing frames?
    // TODO : 테스트 필요
    _videoOutput.alwaysDiscardsLateVideoFrames = YES;
    
    // 4.2. We want the frames in some RGB format
    NSNumber* framePixelFormat = [ NSNumber numberWithInt: kCVPixelFormatType_32BGRA ];
    //    NSNumber* framePixelFormat = [ NSNumber numberWithInt: kCVPixelFormatType_420YpCbCr8BiPlanarFullRange ];
    _videoOutput.videoSettings = [ NSDictionary dictionaryWithObject: framePixelFormat
                                                              forKey: ( id ) kCVPixelBufferPixelFormatTypeKey ];
    
    // 5. Add the video data output to the capture session
    [ _captureSession addOutput: _videoOutput ];
    
    if (bCodeScanner==YES) {
        // for qrcode meta data
        AVCaptureMetadataOutput *captureMetadataOutput = [[AVCaptureMetadataOutput alloc] init];
        if ([_captureSession canAddOutput:captureMetadataOutput]) {
            [_captureSession addOutput:captureMetadataOutput];
        }
        
        NSArray *barCodeTypes = @[AVMetadataObjectTypeUPCECode, AVMetadataObjectTypeCode39Code, AVMetadataObjectTypeCode39Mod43Code,
                                  AVMetadataObjectTypeEAN13Code, AVMetadataObjectTypeEAN8Code, AVMetadataObjectTypeCode93Code, AVMetadataObjectTypeCode128Code,
                                  AVMetadataObjectTypePDF417Code, AVMetadataObjectTypeQRCode, AVMetadataObjectTypeAztecCode, AVMetadataObjectTypeInterleaved2of5Code,
                                  AVMetadataObjectTypeITF14Code, AVMetadataObjectTypeDataMatrixCode, AVMetadataObjectTypeFace];
        
        [captureMetadataOutput setMetadataObjectsDelegate:self queue:_sessionQueue];
        [captureMetadataOutput setMetadataObjectTypes:barCodeTypes];
    }
}


- ( void ) setupStillImageOutput
{
    _stillImageOutput = [ [ AVCaptureStillImageOutput alloc ] init ];
    
    NSDictionary* outputSettings = @{ AVVideoCodecKey: AVVideoCodecJPEG };
    [ _stillImageOutput setOutputSettings:outputSettings ];
    
    // 테스트
    #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    [ _stillImageOutput setHighResolutionStillImageOutputEnabled:YES ];
    #endif
    
    if ([ _captureSession canAddOutput: _stillImageOutput ]) {
        [ _captureSession addOutput: _stillImageOutput ];
    }
}

- ( BOOL ) isFrontFacingCameraPresent
{
    #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    NSArray* devices = [ AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo ];
    
    for (AVCaptureDevice* device in devices)
    {
        if ([ device position ] == AVCaptureDevicePositionFront)
            return YES;
    }
    
    return NO;
    #else
    return YES;
    #endif
}

- ( BOOL ) isFlashPresent
{
    #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    return [_cameraDevice hasFlash];
    #else
    return NO;
    #endif
}

- ( BOOL ) isFocusModeSupport
{
    #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    return  [_cameraDevice isFocusModeSupported:AVCaptureFocusModeLocked] ||
    [_cameraDevice isFocusModeSupported:AVCaptureFocusModeAutoFocus] ||
    [_cameraDevice isFocusModeSupported:AVCaptureFocusModeContinuousAutoFocus];
    #else
    return NO;
    #endif
}

- ( void ) videoCameraStarted: ( NSNotification* ) note
{
    // This callback has done its job, now disconnect it
    [ [ NSNotificationCenter defaultCenter ] removeObserver: self
                                                       name: AVCaptureSessionDidStartRunningNotification
                                                     object: _captureSession ];
    
    // Now send an event to ActionScript
    if (_cameraPlayer) {
        _cameraPlayer->onCameraStarted(_cameraId);
    }
    
    #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    // silent autofocus
    [ self autoFocusAtPoint: CGPointMake(0.5, 0.5) ];
    #endif
}

- ( void ) observeValueForKeyPath: (NSString *)keyPath ofObject: (id)object change: (NSDictionary *)change context: (void *)context {
    // continuous autofocus에서 이걸 자꾸 빼먹는다.
    // continuous만 하지 말고 그냥 autofocus를 먼저하고 그다음에 continuous를 해야할 듯...
    
}

- ( BOOL ) startCameraInternal: (int) cameraId
{
    // 1. Find the back camera
    if ( ![ self findCamera: cameraId ] )
    {
        return FALSE;
    }
    
    //2. Make sure we have a capture session
    if ( NULL == _captureSession )
    {
        _captureSession = [ [ AVCaptureSession alloc ] init ];
    }
    
    // 3. Choose a preset for the session.
    NSString* cameraResolutionPreset = AVCaptureSessionPreset1280x720;
    
    // 4. Check if the preset is supported on the device by asking the capture session:
    if ( ![ _captureSession canSetSessionPreset: cameraResolutionPreset ] )
    {
        cameraResolutionPreset = AVCaptureSessionPreset640x480;
        if ( ![ _captureSession canSetSessionPreset: cameraResolutionPreset ] )
        {
            return FALSE;
        }
    }
    
    // 4.1. The preset is OK, now set up the capture session to use it
    [ _captureSession setSessionPreset: cameraResolutionPreset ];
    
    // 5. Plug camera and capture sesiossion together
    if (![ self attachCameraToCaptureSession ])
    {   // 카메라 설정 비허용 상태
        return FALSE;
    }
    
    // 6. Add the video output
    [ self setupVideoOutput ];
    
    // Add the still image output
    [ self setupStillImageOutput ];
    
    // 7. Set up a callback, so we are notified when the camera actually starts
    [ [ NSNotificationCenter defaultCenter ] addObserver: self
                                                selector: @selector( videoCameraStarted: )
                                                    name: AVCaptureSessionDidStartRunningNotification
                                                  object: _captureSession ];
    
    
    #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    [ _cameraDevice addObserver: self forKeyPath: @"adjustingFocus" options: NSKeyValueObservingOptionNew context: nil ];
    [ _cameraDevice addObserver: self forKeyPath: @"adjustingExposure" options: NSKeyValueObservingOptionNew context: nil ];
    #endif
    
    // 8. 3, 2, 1, 0... Start!
    [ _captureSession startRunning ];
    
    // Note: Returning true from this function only means that setting up went OK.
    // It doesn't mean that the camera has started yet.
    // We get notified about the camera having started in the videoCameraStarted() callback.
    return TRUE;
}

- ( void ) stopCameraInternal
{
    if ( NULL == _captureSession )
    {
        // The camera was never started, don't bother stpping it
        return;
    }
    
    // Make sure we don't pull the rug out of the camera thread's feet.
    // Get hold of a mutex with @synchronized and then stop
    // and tidy up the capture session.
    @synchronized( self )
    {
        if ( [ _captureSession isRunning ] )
        {
            #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
            [ _cameraDevice removeObserver: self forKeyPath: @"adjustingFocus" ];
            [ _cameraDevice removeObserver: self forKeyPath: @"adjustingExposure" ];
            #endif
            
            [ _captureSession stopRunning ];
            
            assert( NULL != _videoOutput );
            [ _captureSession removeOutput: _videoOutput ];
            
            assert( NULL != _cameraInput );
            [ _captureSession removeInput: _cameraInput ];
            
            _captureSession = NULL;
            _cameraDevice = NULL;
            _cameraInput = NULL;
            _videoOutput = NULL;
            
            while (auto frame = [ self dequeueDecodeFrame ]) {
                [ self queueIdleFrame:frame ];
            }
            if (_cameraPlayer) {
                _cameraPlayer->onCameraStopped();
            }
            
            _frameReceived = 0;
        }
    }
}

- ( BOOL ) startCamera : ( int ) cameraId
{
    dispatch_async( _sessionQueue, ^{
        int reqCameraId;
        if (cameraId == CameraDeviceConst::Facing::FRONT) {
            reqCameraId = CameraDeviceConst::Facing::FRONT;
        } else {
            reqCameraId = CameraDeviceConst::Facing::BACK;
        }
        
        if (![ self startCameraInternal: reqCameraId ]) {
            if (_cameraPlayer) {
                _cameraPlayer->onCameraAccessDenied();
            }
        }
    });
    
    return TRUE;
}

- ( void ) stopCamera
{
    dispatch_async( _sessionQueue, ^{
        [ self stopCameraInternal ];
    });
}

- ( BOOL ) switchCamera
{
    if ( [ _captureSession isRunning ] )
    {
        dispatch_async( _sessionQueue, ^{
            int cameraId;
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
            if (_cameraId != CameraDeviceConst::Facing::BACK) {
                cameraId = CameraDeviceConst::Facing::BACK;
            } else {
                cameraId = CameraDeviceConst::Facing::FRONT;
            }
#else
            cameraId = CameraDeviceConst::Facing::FRONT;
#endif
            
            [ self stopCameraInternal ];
            [ self startCameraInternal: cameraId ];
        });
        
        return TRUE;
    }
    return FALSE;
}

- ( void ) cleanup
{
    dispatch_async( _sessionQueue, ^{
        [ self stopCamera ];
        _cameraPlayer.reset();
    });
}



//-------------------------------------------------------------------------------------
- ( VideoFrame* const ) dequeueIdleFrame
{
    VideoFrame* frame = NULL;
    
    [ _queueLock lock ];
    {
        if (!_idleQueue.empty()) {
            frame = _idleQueue.front();
            _idleQueue.pop();
        }
    }
    [ _queueLock unlock ];
    
    return frame;
}

- ( void ) queueIdleFrame: (VideoFrame*) frame
{
    [ _queueLock lock ];
    {
        _idleQueue.push(frame);
    }
    [ _queueLock unlock ];
}


- ( VideoFrame* const ) dequeueDecodeFrame
{
    VideoFrame* frame = NULL;
    
    [ _queueLock lock ];
    {
        if (!_decodeQueue.empty()) {
            frame = _decodeQueue.front();
            _decodeQueue.pop();
        }
    }
    [ _queueLock unlock ];
    
    return frame;
}

- ( void ) queueDecodeFrame: (VideoFrame*) frame
{
    [ _queueLock lock ];
    {
        _decodeQueue.push(frame);
    }
    [ _queueLock unlock ];
}

- ( BOOL ) isFrameReceived
{
    size_t size = 0;
    [ _queueLock lock ];
    {
        size = _decodeQueue.size();
    }
    [ _queueLock unlock ];
    
    return size > 0;
}

- (NSArray *)translatePoints:(NSArray *)points fromView:(UIView *)fromView toView:(UIView *)toView
{
    NSMutableArray *translatedPoints = [NSMutableArray new];
    
    // The points are provided in a dictionary with keys X and Y
    for (NSDictionary *point in points) {
        // Let's turn them into CGPoints
        CGPoint pointValue = CGPointMake([point[@"X"] floatValue], [point[@"Y"] floatValue]);
        // Now translate from one view to the other
        CGPoint translatedPoint = [fromView convertPoint:pointValue toView:toView];
        // Box them up and add to the array
        [translatedPoints addObject:[NSValue valueWithCGPoint:translatedPoint]];
    }
    
    return [translatedPoints copy];
}

//-------------------------------------------------------------------------------------

-(void)captureOutput:(AVCaptureOutput *)captureOutput didOutputMetadataObjects:(NSArray *)metadataObjects fromConnection:(AVCaptureConnection *)connection{
    // qr 코드 일때...
    if (metadataObjects != nil && [metadataObjects count] > 0) {
        AVMetadataMachineReadableCodeObject *metadataObj = [metadataObjects objectAtIndex:0];
        if (![metadataObj respondsToSelector:@selector(stringValue)]) {
            NSLog(@"[[[[[ WTF code scane string value unrecognized....????? !!!!!");
            return;
        }
        NSString * nsResult = [metadataObj stringValue];
        NSArray<NSDictionary *> * corners = [metadataObj corners];

        if (corners.count!=4) {
            return;
        }
     
        if (frameWidth==-1) {
            return;
        }
        
//        dispatch_async(dispatch_get_main_queue(), ^{
            std::vector<float> pts;
            pts.clear();
            for (NSDictionary * dic in corners) {
                CGPoint pointValue = CGPointMake([dic[@"X"] floatValue], [dic[@"Y"] floatValue]);
                
                pts.push_back(pointValue.x*frameWidth-cropHalfWidth/2);
                pts.push_back(frameHeight - pointValue.y*frameHeight-cropHalfHeight/2);
            }
            
            _cameraPlayer->detectedScanCode(pts, 0, nsResult.UTF8String);
            
//        });

    }
}

// camera 영상이 실시간으로 나오는 Delegate
- ( void ) captureOutput: ( AVCaptureOutput* ) captureOutput
   didOutputSampleBuffer: ( CMSampleBufferRef ) sampleBuffer
          fromConnection: ( AVCaptureConnection* ) connection
{
    // 1. Check if this is the output we are expecting:
    if ( captureOutput == _videoOutput )
    {
        // 2. If it's a video frame, copy it from the sample buffer:
        [ self copyVideoFrame: sampleBuffer ];
    }
}

// 매 프레임을 버퍼에 저장하여 큐에 쌓는다.
- ( void ) copyVideoFrame: ( CMSampleBufferRef ) sampleBuffer
{
    auto frame = [ self dequeueIdleFrame ];
    if (!frame) {
        // 대기중인 버퍼 없음.
        return;
    }
    
    // 1. Get a pointer to the pixel buffer:
    CVPixelBufferRef pixelBuffer = ( CVPixelBufferRef ) CMSampleBufferGetImageBuffer( sampleBuffer );
    
    // 2. Obtain access to the pixel buffer by locking its base address:
    CVOptionFlags lockFlags = 0; // If you are curious, look up the definition of CVOptionFlags
    CVReturn status = CVPixelBufferLockBaseAddress( pixelBuffer, lockFlags );
    assert( kCVReturnSuccess == status );
    
    // 3. Copy bytes from the pixel buffer
    // 3.1. First, work out how many bytes we need to copy:
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow( pixelBuffer );
    size_t width = CVPixelBufferGetWidth( pixelBuffer );
    size_t height = CVPixelBufferGetHeight( pixelBuffer );
    size_t length = bytesPerRow * height;
    
    
    // 3.2. Then work out where in memory we'll need to start copying:
    void* baseAddr = CVPixelBufferGetBaseAddress( pixelBuffer );
    
    size_t cropWidth = (4 * height / 3);
    size_t cropLength = cropWidth * height * 4;
    
    bool needCrop = false;
    if (cropWidth != width) {
        // 4 : 3 크롭
        needCrop = true;
    }
    
    // TODO : 버퍼 메모리 절약을 위해 YUV포맷으로 테스트 필요
    if (frame->buffer == nullptr) {
        // 버퍼 생성
        if (needCrop) {
            frame->buffer = (uint8_t*)malloc(cropLength);
            frame->length = cropLength;
            frame->width = (int)cropWidth;
            frame->height = (int)height;
            
            // copy to frame buffer
            size_t offset = (width - cropWidth)/2;
            size_t rowBytes = cropWidth * 4;
            
            uint32_t* src = (uint32_t*)baseAddr + offset;
            uint32_t* dst = (uint32_t*)frame->buffer;
            
            for (int i = 0; i < height; i++) {
                memcpy(dst, src, rowBytes);
                dst += cropWidth;
                src += width;
            }
        } else {
            frame->buffer = (uint8_t*)malloc(length);
            frame->length = length;
            frame->width = (int)width;
            frame->height = (int)height;
            
            // copy to buffer
            memcpy(frame->buffer, baseAddr, length);
        }
    } else {
        if (needCrop) {
            if (frame->length != cropLength) {
                // 버퍼 크기가 다르면 realloc
                frame->buffer = (uint8_t*)realloc((void*)frame->buffer, cropLength);
                frame->length = cropLength;
                frame->width = (int)cropWidth;
                frame->height = (int)height;
            }
            
            // copy to frame buffer
            size_t offset = (width - cropWidth)/2;
            size_t rowBytes = cropWidth * 4;
            
            uint32_t* src = (uint32_t*)baseAddr + offset;
            uint32_t* dst = (uint32_t*)frame->buffer;
            
            for (int i = 0; i < height; i++) {
                memcpy(dst, src, rowBytes);
                dst += cropWidth;
                src += width;
            }
            
        } else {
            if (frame->length != length) {
                // 버퍼 크기가 다르면 realloc
                frame->buffer = (uint8_t*)realloc((void*)frame->buffer, length);
                frame->length = length;
                frame->width = (int)width;
                frame->height = (int)height;
            }
            memcpy(frame->buffer, baseAddr, length);
        }
    }
    
    frameWidth = width;
    frameHeight = height;
    cropHalfWidth = width - frame->width;
    cropHalfHeight = height - frame->height;

    // 6. Let go of the access to the pixel buffer by unlocking the base address:
    CVOptionFlags unlockFlags = 0; // If you are curious, look up the definition of CVOptionFlags
    CVPixelBufferUnlockBaseAddress( pixelBuffer, unlockFlags );
    
    // 디코드 큐에 넣음
    [ self queueDecodeFrame:frame ];
    
    if (_frameReceived == 0 && _cameraPlayer) {
        _cameraPlayer->onCameraFirstFrameReceived(_cameraId, (int)cropWidth, (int)height, (int)width, (int)height);
    }
    
    _frameReceived++;
}

// shutter를 눌러서 snapshot을 찍을때...
- ( void ) captureImageInternal : (int) flashState
{
    if ([ self isFlashPresent ]) {
        
        AVCaptureFlashMode flashMode;
        
        switch (flashState) {
            default:
            case CameraDeviceConst::AUTO:
                flashMode = AVCaptureFlashModeAuto;
                break;
            case CameraDeviceConst::ON:
                flashMode = AVCaptureFlashModeOn;
                break;
            case CameraDeviceConst::OFF:
                flashMode = AVCaptureFlashModeOff;
                break;
        }
        
        [ _cameraDevice lockForConfiguration: nil ];
        {
            [ _cameraDevice setFlashMode: flashMode ];
        }
        [ _cameraDevice unlockForConfiguration];
    }
    
    
    AVCaptureConnection* videoConnection = nil;
    for (AVCaptureConnection* connection in [ _stillImageOutput connections ]) {
        for (AVCaptureInputPort* port in [ connection inputPorts ]) {
            if ([ [ port mediaType ] isEqual:AVMediaTypeVideo ] ) {
                videoConnection = connection;
                break;
            }
        }
        
        if (videoConnection) {
            break;
        }
    }
    
    if ([videoConnection isVideoOrientationSupported]) {
        [videoConnection setVideoOrientation:AVCaptureVideoOrientationPortrait];
    }
    
    [ _stillImageOutput captureStillImageAsynchronouslyFromConnection: videoConnection completionHandler: ^(CMSampleBufferRef sampleBuffer, NSError* error) {
        
        if (_stopCameraAfterCapture) {
            [ self stopCameraInternal ];
        }
        
        /*
         // Exif 필요할 경우 추가.
         CFDictionaryRef exifAttachments = (__bridge CFDictionaryRef)CMGetAttachment(sampleBuffer, kCGImagePropertyExifDictionary, NULL);
         NSDictionary* exifDict = (NSDictionary *)exifAttachments;
         for (id key in exifDict) {
         NSLog(@"key = %@, value = %@",key,[exifDict objectForKey:key]);
         }
         */
        
        NSData* data = [ AVCaptureStillImageOutput jpegStillImageNSDataRepresentation: sampleBuffer ];
        if (data && [ data bytes ] && [ data length ] > 0) {
            // success
            _cameraPlayer->onCameraCaptureComplete((uint8_t*)[ data bytes ], (size_t)[ data length ]);
            
        } else {
            // failed
            _cameraPlayer->onCameraCaptureComplete(nullptr, 0);
        }
    }];
}

- ( void ) captureImage : ( int ) flashState flag: ( bool ) stopAfterCapture
{
    dispatch_async( _sessionQueue, ^{
        _stopCameraAfterCapture = stopAfterCapture;
        [ self captureImageInternal: flashState ];
    });
}

- ( BOOL ) updateTexture: (cocos2d::Texture2D*) texture
{
    auto frame = [ self dequeueDecodeFrame ];
    
    if (!frame) {
        // 저장된 frame 없음.
        return FALSE;
    }
    
    if (_cameraPlayer) {
        _cameraPlayer->onFrameRendered(frame);
    }

    cocos2d::Size size = texture->getContentSizeInPixels();
    size_t length = frame->length;
    
    if (size.width != frame->width || size.height != frame->height) {
        // 기존과 크기가 다르면 texure 초기화
        texture->initWithData(frame->buffer, length, cocos2d::Texture2D::PixelFormat::RGBA8888, frame->width, frame->height, cocos2d::Size(frame->width, frame->height));
    } else {
        texture->updateWithData(frame->buffer, 0, 0, frame->width, frame->height);
    }
    
    [ self queueIdleFrame:frame ];
    
    return TRUE;
}


//-------------------------------------------------------------------------------------
- ( bool ) autoFocusAtPoint: (CGPoint) point
{
    #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    bool ret = false;
    if ( [ _captureSession isRunning ] ) {
        NSError *error;
        // focus
        if ([ _cameraDevice isFocusModeSupported: AVCaptureFocusModeContinuousAutoFocus ] ||
            [ _cameraDevice isFocusModeSupported: AVCaptureFocusModeAutoFocus ] ||
            [ _cameraDevice isFocusModeSupported: AVCaptureFocusModeLocked ]) {
            
            if ([ _cameraDevice lockForConfiguration: &error ]) {
                ret = true;
                
                if ([ _cameraDevice isFocusPointOfInterestSupported ]) {
                    [ _cameraDevice setFocusPointOfInterest: point];
                }
                
                if ([ _cameraDevice isFocusModeSupported: AVCaptureFocusModeContinuousAutoFocus ]) {
                    [ _cameraDevice setFocusMode: AVCaptureFocusModeContinuousAutoFocus ];
                } else if ([ _cameraDevice isFocusModeSupported: AVCaptureFocusModeAutoFocus ]) {
                    [ _cameraDevice setFocusMode: AVCaptureFocusModeAutoFocus ];
                } else {
                    [ _cameraDevice setFocusMode: AVCaptureFocusModeLocked ];
                }
                
                [ _cameraDevice unlockForConfiguration ];
            } else {
                NSLog(@"[[[[[ AVCaptureDevice Error : %@", [ error description ]);
            }
        }
        
        // exposure
        if ([ _cameraDevice isExposureModeSupported: AVCaptureExposureModeAutoExpose ]) {
            if ([_cameraDevice lockForConfiguration: &error]) {
                ret = true;
                
                if ([ _cameraDevice isFocusPointOfInterestSupported ]) {
                    [ _cameraDevice setExposurePointOfInterest: point ];
                }
                [ _cameraDevice setExposureMode: AVCaptureExposureModeAutoExpose ];
                
                [ _cameraDevice unlockForConfiguration ];
            } else {
                NSLog(@"[[[[[ AVCaptureDevice Error : %@", [ error description ]);
            }
        }
    }
    return ret;
    #else
    return true;
    #endif
}

@end
