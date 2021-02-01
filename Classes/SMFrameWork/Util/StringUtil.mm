//
//  StringUtil.m
//  IMKSP
//
//  Created by SteveMac on 2017. 10. 31..
//

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#import "StringUtil.h"

#import <Foundation/Foundation.h>

static NSString * UUID = @"";
static NSString * UDID = @"";



@interface IOSString : NSObject
{
    
}
+ (NSString*)getUUID;
+ (NSString*)getUDID;
- (NSString*)getFakeUDID;
@end

@implementation IOSString

- (id)init
{
    if (self=[super init]) {

    }
    return self;
}

+ (NSString*)getUUID
{
    if ([UUID isEqualToString:@""]) {
        CFUUIDRef uuidRef = CFUUIDCreate(NULL);
        
        CFStringRef uuidStringRef = CFUUIDCreateString(NULL, uuidRef);

        UUID = (__bridge NSString *)uuidStringRef;
    }
    return UUID;
}

+ (NSString*)getUDID
{
    IOSString * iosstring = [[IOSString alloc] init];
    return [iosstring getFakeUDID];
}

- (NSString*)getFakeUDID
{
    return @"";
}

@end


std::string StringUtil::getDeviceCurrentUinqueID()
{
    NSString * uuidString = [IOSString getUUID];
    std::string retString = std::string([uuidString UTF8String]);
    return retString;
}

std::string StringUtil::getDeviceForeverUniqueID()
{
    NSString * udidString = [IOSString getUDID];
    std::string retString = std::string([udidString UTF8String]);
    return retString;
}

#endif
