//
//  AppleCamera.h
//  iPet
//
//  Created by KimSteve on 2017. 7. 5..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DeviceCamera.h"

@interface AppleCamera : NSObject
- ( id ) initWithHost : (std::shared_ptr<CameraPlayer>) player withCodeScanner:(BOOL) codeScanner;
- (NSArray *)translatePoints:(NSArray *)points fromView:(UIView *)fromView toView:(UIView *)toView;
- ( BOOL ) startCamera : ( int ) cameraId;
- ( BOOL ) isFrontFacingCameraPresent;
- ( BOOL ) isFlashPresent;
- ( BOOL ) isFocusModeSupport;
- ( void ) stopCamera;
- ( BOOL ) switchCamera;
- ( void ) captureImage : ( int ) flashState flag: ( bool ) stopAfterCapture;
- ( BOOL ) updateTexture: (cocos2d::Texture2D*) texture;
- ( BOOL ) isFrameReceived;

// video frame queue
- ( void ) queueIdleFrame: (VideoFrame*) frame;
- ( VideoFrame* const ) dequeueIdleFrame;
- ( void ) queueDecodeFrame: (VideoFrame*) frame;
- ( VideoFrame* const ) dequeueDecodeFrame;
- ( bool ) autoFocusAtPoint: (CGPoint) point;
- ( void ) cleanup;

@end
