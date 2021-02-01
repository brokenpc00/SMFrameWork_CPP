//
//  FileManager.m
//  ePubCheck
//
//  Created by SteveMac on 2018. 9. 10..
//

#include "FileManager.h"
#include <string>


#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
bool FileManager::copyFile(std::string src, std::string dst)
{
    NSString *srcPath = [NSString stringWithUTF8String:src.c_str()];
    NSString *dstPath = [NSString stringWithUTF8String:dst.c_str()];
    
    NSError* error = nil;
    if ( ! [[NSFileManager defaultManager] copyItemAtPath:srcPath toPath:dstPath error:&error] ) {
        NSLog(@"Could not copy file: %@ -> %@", srcPath, dstPath);
        NSLog(@"copyItemAtPath error: %@", error);
        return false;
    }
    return true;
}

bool FileManager::createFolder(std::string foldername)
{
    NSString* fullpath = [NSString stringWithUTF8String:foldername.c_str()];
    
    NSError *error = nil;
    if ( ! [[NSFileManager defaultManager] createDirectoryAtPath:fullpath
                                     withIntermediateDirectories:YES
                                                      attributes:nil
                                                           error:&error]) {
        NSLog(@"Could not create folder: %@", fullpath);
        NSLog(@"createDirectoryAtPath error: %@", error);
        return false;
    }
    
    return true;
}

bool FileManager::deleteFile(std::string filename )
{
    NSString* fullpath = [NSString stringWithUTF8String:filename.c_str()];
    
    NSError *error = nil;
    if ( ! [[NSFileManager defaultManager] removeItemAtPath:fullpath error:&error] ){
        NSLog(@"Could not delete file: %@", fullpath);
        NSLog(@"removeItemAtPath error: %@", error);
        return false;
    }
    
    return true;
}
#endif
