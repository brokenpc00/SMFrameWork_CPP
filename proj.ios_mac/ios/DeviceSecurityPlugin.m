//
//  DeviceSecurityPlugin.m
//  ECheckForm
//
//  Created by SteveKim on 2019/10/28.
//

#import "DeviceSecurityPlugin.h"
#include <sys/sysctl.h>
#include <sys/utsname.h>

typedef NS_ENUM(NSUInteger, UIDeviceType) {
    DevicePhone5LessType = 0,
    DevicePhone6LessType,
    DevicePhone6Type,
    DevicePhone6PlusType,
    DevicePhoneXType,
    DevicePhoneXRType,
    DevicePhoneXMaxType,
    DeviceUnknown = 999,
};

@interface UIDevice(ECHECKExtend)
+(UIDeviceType)getDeviceType;

+(NSString *)deviceMachine;
@end
#define JAILBROKEN_MESSAGE @"이 디바이스는 JailBroken되어 있습니다. 이 디바이스에서는 서비스를 이용하실 수 없습니다. 정상적인 디바이스에서 다시 실행하세요."

#define JAILBROKEN_JSON  @"{\
\"application\" : [ \
                 \"/Applications/blackra1n.app\",\
                 \"/Applications/Cydia.app\",\
                 \"/Applications/FakeCarrier.app\",\
                 \"/Applications/Icy.app\",\
                 \"/Applications/IntelliScreen.app\",\
                 \"/Applications/MxTube.app\",\
                 \"/Applications/RockApp.app\",\
                 \"/Applications/SBSettings.app\",\
                 \"/Applications/WinterBoard.app\"\
                 ],\
\"path\" : [\
          \"/bin/ps\",\
          \"/etc/master.passwd\",\
          \"/Library/MobileSubstrate/DynamicLibraries/LiveClock.plist\",\
          \"/Library/MobileSubstrate/DynamicLibraries/Veency.plist\",\
          \"/private/var/lib/apt\",\
          \"/private/var/stash\",\
          \"/private/var/mobile/Library/SBSettings/Themes\",\
          \"/private/var/tmp/cydia.log\",\
          \"/private/var/lib/cydia\",\
          \"/System/Library/LaunchDaemons/com.ikey.bbot.plist\",\
          \"/System/Library/LaunchDaemons/com.saurik.Cydia.Startup.plist\",\
          \"/usr/sbin/sshd\",\
          \"/usr/bin/sshd\",\
          \"/usr/libexec/sftp-server\"\
          ]\
}"

@interface DeviceSecurityPlugin (){
    NSDictionary *_jailbreakList;
}

@property(nonatomic, strong) NSDictionary *jailbreakList;



@end

@implementation DeviceSecurityPlugin

+ (DeviceSecurityPlugin *)sharedInstance{
    
    static DeviceSecurityPlugin *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[DeviceSecurityPlugin alloc] init];
    });
    
    return sharedInstance;
}
-(id)init
{
    self = [super init];
    if(self) {
   
    }
    return self;
}

- (BOOL)isJailBroken{
    
    if([[UIDevice deviceMachine] isEqual:@"x86_64"]){
        return NO;
    }
    self.jailbreakList = [self getJailbreakList];
    
    if (![self level1]) {
        [self showAlertMessage:JAILBROKEN_MESSAGE];
        return YES;
    }
    
    if (![self level2]) {
        [self showAlertMessage:JAILBROKEN_MESSAGE];
        return YES;
    }
    return NO;
}

- (void)start{
    
    if([[UIDevice deviceMachine] isEqual:@"x86_64"]){
        return;
    }
    self.jailbreakList = [self getJailbreakList];
    
    if (![self level1]) {
        [self showAlertMessage:JAILBROKEN_MESSAGE];
        return;
    }
    
    if (![self level2]) {
        [self showAlertMessage:JAILBROKEN_MESSAGE];
        return;
    }
}

//API 호출 기반 탐지
-(BOOL) level1{
    
    NSLog(@"level1");
    
    NSArray *applicationList = [self.jailbreakList objectForKey:@"application"];

    for (int i=0; i<applicationList.count; i++) {
        
//        NSLog(@"target = %@", [applicationList objectAtIndex:i]);
        
        if ([[NSFileManager defaultManager] fileExistsAtPath:[applicationList objectAtIndex:i]] == YES) {
            
            return NO;
            
        }
        
    }
    
    NSArray *pathList = [self.jailbreakList objectForKey:@"path"];
    
    for (int i=0; i<pathList.count; i++) {
        
//        NSLog(@"target = %@", [pathList objectAtIndex:i]);
        
        if ([[NSFileManager defaultManager] fileExistsAtPath:[pathList objectAtIndex:i]] == YES) {
            
            return NO;
            
        }
        
    }
    
    return YES;
}

//시스템 호출 기반 탐지
- (BOOL)level2{
    
    NSLog(@"level2");
    
    NSArray *applicationList = [self.jailbreakList objectForKey:@"application"];

    for (int i=0; i<[applicationList count]; i++) {
        
//        NSLog(@"target = %@", [applicationList objectAtIndex:i]);
        
        const char* str = [[applicationList objectAtIndex:i] UTF8String];
        
        if(open(str ,O_RDONLY) != -1){
            
            return NO;
        }
        
    }
    
    NSArray *pathList = [self.jailbreakList objectForKey:@"path"];
    
    for (int i=0; i<[pathList count]; i++) {
        
//        NSLog(@"target = %@", [pathList objectAtIndex:i]);
        
        const char* str = [[pathList objectAtIndex:i] UTF8String];
        
        if(open(str ,O_RDONLY) != -1){
            
            return NO;
        }
        
    }
    
    return YES;
}

- (NSDictionary *)getJailbreakList{
    
//    NSString *path = [[NSBundle mainBundle] pathForResource:@"jailbreak_list.json" ofType:nil];
//
    NSDictionary *jailbreakList = [NSDictionary dictionary];
//
//    if(![[NSFileManager defaultManager] fileExistsAtPath:path]){
//        jailbreakList = nil;
//    }
    
//    NSString *strConfigInfo =[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:NULL];
//
//    NSData* data = [strConfigInfo dataUsingEncoding:NSUTF8StringEncoding];
    
    
    NSString *strConfigInfo =JAILBROKEN_JSON;
    
    NSData* data = [strConfigInfo dataUsingEncoding:NSUTF8StringEncoding];
    
    jailbreakList = [NSJSONSerialization JSONObjectWithData:data
                                                    options:NSJSONReadingMutableContainers
                                                      error:nil];
    
    return jailbreakList;
}

- (void)showAlertMessage:(NSString*)message{

    UIAlertView* customAlert = [[UIAlertView alloc] initWithTitle:nil message:message delegate:nil cancelButtonTitle:nil otherButtonTitles:nil];
    customAlert.delegate = self;
    [customAlert show];

}

-(void)dealloc{
    
    self.jailbreakList = nil;
}

@end

@implementation UIDevice(ECHECKExtend)

+(UIDeviceType)getDeviceType{
    
    if([UIScreen mainScreen].bounds.size.width==320){
        
        if([UIScreen mainScreen].bounds.size.height < 568){
            return DevicePhone5LessType;
        }
        return DevicePhone6LessType;
    }
    else if([UIScreen mainScreen].bounds.size.width==375){
        if([UIScreen mainScreen].bounds.size.height == 812){
            return DevicePhoneXType;
        }
        return DevicePhone6Type;
    }
    else{
        if([UIScreen mainScreen].bounds.size.height == 896){
            return DevicePhoneXMaxType;
        }
        return DevicePhone6PlusType;
    }
    
    return DeviceUnknown;
}
+ (NSString *)deviceMachine{
    
    struct utsname u;
    
    uname(&u);
    
    NSString * deviceValue = [NSString stringWithUTF8String:u.machine];
    
    return deviceValue;
}

@end
