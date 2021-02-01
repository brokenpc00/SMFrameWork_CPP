//
//  DeviceSecurityPlugin.h
//  ECheckForm
//
//  Created by SteveKim on 2019/10/28.
//

#import <Foundation/Foundation.h>


@interface DeviceSecurityPlugin : NSObject

+ (DeviceSecurityPlugin *)sharedInstance;

- (BOOL)isJailBroken;
- (void)start;
- (BOOL)level1;
- (BOOL)level2;
- (NSDictionary *)getJailbreakList;
- (void)showAlertMessage:(NSString*)message;

@end
