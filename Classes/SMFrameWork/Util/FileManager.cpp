//
//  FileManager.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 17..
//
//

#include "FileManager.h"
#include <stdio.h>
#include <dirent.h>

static FileManager* _instance = nullptr;


const int FileManager::SUCCESS = 0;
const int FileManager::FAILED = -1;

// app main data root path
static const char* DATA_ROOT_PATH = "Data/";

static const char* IMAGE_ROOT = "Images/";
static const char* DOC_ROOT = "Doc/";
static const char* SNAPSHOT_ROOT = "SnapShot/";
static const char* ZIP_ROOT = "Zip/";
static const char* XML_ROOT = "XML/";
static const char* PRELOAD_ROOT = "Preload/";
static const char* DB_ROOT = "DB/";
static const char* EPUB_DOWN_ROOT = "brokenpcEBook/Downloads/";
static const char* EPUB_EXTRACT_ROOT = "brokenpcEBook/Extract/";

FileManager* FileManager::getInstance()
{
    if (_instance==nullptr) {
        _instance = new (std::nothrow) FileManager();
        if (!_instance->init()) {
            CC_SAFE_DELETE(_instance);
        }
    }
    
    CCASSERT(_instance!=nullptr, "Failed to create file manager");
    return _instance;
}

FileManager::FileManager()
{
    
}

FileManager::~FileManager()
{
    
}

bool FileManager::init()
{
    auto fileUtil = cocos2d::FileUtils::getInstance();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    auto rootPath = fileUtil->getWritablePath().append(DATA_ROOT_PATH).append("MACAPP_DATA/");
#else
    auto rootPath = fileUtil->getWritablePath().append(DATA_ROOT_PATH);
#endif
    if (!fileUtil->isDirectoryExist(rootPath)) {
        // not exist... so... create new data root path
        fileUtil->createDirectory(rootPath);
    }
    
    getFullPath(FileType::Image);
    getFullPath(FileType::Doc);
    getFullPath(FileType::SnapShot);
    getFullPath(FileType::ZIP);
    getFullPath(FileType::XML);
    getFullPath(FileType::Preload);
    getFullPath(FileType::DB);
    getFullPath(FileType::EPUB_DOWN);
    getFullPath(FileType::EPUB_EXTRACT);
    
    return true;
}

bool FileManager::isFileEixst(const FileManager::FileType type, const std::string &fileName)
{
    return cocos2d::FileUtils::getInstance()->isFileExist(getFullFilePath(type, fileName));
}

bool FileManager::writeToFile(const FileManager::FileType type, const std::string &fileName, u_char *buffer, size_t bufSize)
{
    std::lock_guard<std::mutex> guard(_mutex);
    
    auto fileUtil = cocos2d::FileUtils::getInstance();
    auto fullPath = getFullFilePath(type, fileName);
    
    if (buffer==nullptr || bufSize==0) {
        return false;
    }
        
    cocos2d::Data fileData;
    fileData.fastSet(buffer, bufSize);
    
    bool bSuccess = fileUtil->writeDataToFile(fileData, fullPath);
    
    fileData.fastSet(nullptr, 0);
    
    return bSuccess;
}

cocos2d::Data FileManager::loadFromFile(const FileManager::FileType type, const std::string &fileName, int *error)
{
    auto fileUtil = cocos2d::FileUtils::getInstance();
    auto fullPath = getFullFilePath(type, fileName);
    
    *error = FAILED;
    if (fileUtil->isFileExist(fullPath)) {
        *error = SUCCESS;
        return fileUtil->getDataFromFile(fullPath);
    }
    
    return cocos2d::Data::Null;
}

std::string FileManager::loadStringFromFile(const FileManager::FileType type, const std::string &fileName, int *error)
{
    auto fileUtil = cocos2d::FileUtils::getInstance();
    auto fullPath = getFullFilePath(type, fileName);
    
    *error = FAILED;
    if (fileUtil->isFileExist(fullPath)) {
        *error = SUCCESS;
        return fileUtil->getStringFromFile(fullPath);
    }
    
    return "";

}

bool FileManager::removeFile(const FileManager::FileType type, const std::string &fileName)
{
    auto fileUtil = cocos2d::FileUtils::getInstance();
    auto fullPath = getFullFilePath(type, fileName);
    
    return fileUtil->removeFile(fullPath);;
}

bool FileManager::renameFile(const FileManager::FileType type, const std::string &oldName, std::string &newName)
{
    return cocos2d::FileUtils::getInstance()->renameFile(getFullPath(type), oldName, newName);
}

std::string FileManager::getFullFilePath(const FileManager::FileType type, const std::string &fileName)
{
    return getFullPath(type).append(fileName);
}

std::string FileManager::getLocalFilePath(const FileManager::FileType type, const std::string &fileName)
{
    return getLocalPath(type).append(fileName);
}

std::string FileManager::getFullPath(const FileManager::FileType type)
{
    auto fileUtil = cocos2d::FileUtils::getInstance();
    auto dir = fileUtil->getWritablePath().append(getLocalPath(type));
    
    if (!fileUtil->isDirectoryExist(dir)) {
        // 디렉토리가 없으면 만든다.
        if (fileUtil->isFileExist(dir)) {
            // 만들려는 디렉토리가 파일하고 이름이 겹치면 파일을 지운다.
            fileUtil->removeFile(dir);
        }
        // 만든다.
        fileUtil->createDirectory(dir);
    }
    
    return dir;
}

std::string FileManager::getLocalPath(const FileManager::FileType type)
{
    const char* baseName;
    
    switch (type) {
        case FileType::Image:
        {
            baseName = IMAGE_ROOT;
        }
            break;
        case FileType::Doc:
        {
            baseName = DOC_ROOT;
        }
            break;
        case FileType::SnapShot:
        {
            baseName = SNAPSHOT_ROOT;
        }
            break;
        case FileType::ZIP:
        {
            baseName = ZIP_ROOT;
        }
            break;
        case FileType::XML:
        {
            baseName = XML_ROOT;
        }
            break;
        case FileType::Preload:
        {
            baseName = PRELOAD_ROOT;
        }
            break;
        case FileType::DB:
        {
            baseName = DB_ROOT;
        }
            break;
        case FileType::EPUB_DOWN:
        {
            baseName = EPUB_DOWN_ROOT;
        }
            break;
        case FileType::EPUB_EXTRACT:
        {
            baseName = EPUB_EXTRACT_ROOT;
        }
            break;
    }
    
    return std::string(DATA_ROOT_PATH).append(baseName);
}

std::string FileManager::createSaveFileName(const std::string& prefix, const std::string& postfix)
{
    time_t now = time(NULL);
    std::tm * ptm = localtime(&now);
    char buffer[20];
    strftime(buffer, 32, "%Y%m%d_%H%M%S", ptm);
    
    return cocos2d::StringUtils::format("%s_%s_%03d_%s", prefix.c_str(), buffer, cocos2d::random(0, 999), postfix.c_str());
}

std::vector<std::string> FileManager::getFileList(const FileType type)
{
    std::string pathName = getFullPath(type);
    std::vector<std::string> fileList;
    
    DIR* dir;
    struct dirent* ent;
    
    dir = opendir(pathName.c_str());
    if (dir==nullptr) {
        perror(pathName.c_str());
        return fileList;
    }
    
    while ((ent = readdir(dir))!=nullptr) {
        fileList.push_back(ent->d_name);
    }
    closedir(dir);
    
    return fileList;
}

void FileManager::clearCache(const FileManager::FileType type)
{
    auto fileList = getFileList(type);
    auto pathName = getFullPath(type);
    auto fileUtil = cocos2d::FileUtils::getInstance();
    
    int numRemoved = 0;
    
    for (int i=0; i<fileList.size(); i++) {
        if (!fileList[i].empty()) {
            std::string fileFullPath = pathName + fileList[i];
            if (fileUtil->removeFile(fileFullPath)) {
                //  지워졌으면 카운트 증가
                numRemoved++;
            }
        }
    }
    
    CCLOG("FILE CLEAR : %d files removed", numRemoved);
}

int FileManager::listFolder(std::string folder, std::vector<std::string>& entries, bool directoriesOnly)
{
    DIR* dp = opendir(folder.c_str());
    if (dp == NULL) {
        return entries.size();
    }
    
    struct dirent *entry = NULL;
    while((entry = readdir(dp))) {
        if ( directoriesOnly && entry->d_type != DT_DIR )
            continue;
        entries.push_back( entry->d_name );
    }
    closedir(dp);
    
    return entries.size();
}

struct MatchPathSeparator
{
    bool operator()( char ch ) const
    {
        return ch == '/';
    }
};

std::string FileManager::getFileName(const std::string fullPath)
{
    return std::string(std::find_if( fullPath.rbegin(), fullPath.rend(), MatchPathSeparator() ).base(), fullPath.end() );
}

std::string FileManager::getFilePath(const std::string fullPath)
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
    size_t pos = fullPath.find_last_of("\\/");
#else
    size_t pos = fullPath.find_last_of("/");
#endif
    return (std::string::npos == pos) ? "" : fullPath.substr(0, pos);
//
//    std::string::const_reverse_iterator pivot = std::find( fullPath.rbegin(), fullPath.rend(), '.' );
//    return pivot == fullPath.rend() ? fullPath : std::string( fullPath.begin(), pivot.base() - 1 );
}


#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <platform/android/jni/JniHelper.h>

#define  CLASS_NAME "com/brokenpc/smframework/fileutil"


bool FileManager::copyFile(std::string src, std::string dst)
{
    cocos2d::JniMethodInfo minfo;
    bool ok = cocos2d::JniHelper::getStaticMethodInfo(minfo, CLASS_NAME, "copyFile", "(Ljava/lang/String;Ljava/lang/String;)Z");
    if ( !ok ) {
        CCLOG("Could not find instance method copyFile");
        return false;
    }
    jstring jstr1 = minfo.env->NewStringUTF( src.c_str() );
    jstring jstr2 = minfo.env->NewStringUTF( dst.c_str() );
    minfo.env->CallStaticBooleanMethod(minfo.classID, minfo.methodID, jstr1, jstr2);
    return true;
}

bool FileManager::createFolder(std::string foldername)
{
    cocos2d::JniMethodInfo minfo;
    bool ok = cocos2d::JniHelper::getStaticMethodInfo(minfo, CLASS_NAME, "createFolder", "(Ljava/lang/String;)Z");
    if ( !ok ) {
        CCLOG("Could not find instance method createFolder");
        return false;
    }
    jstring jstr = minfo.env->NewStringUTF( foldername.c_str() );
    minfo.env->CallStaticBooleanMethod(minfo.classID, minfo.methodID, jstr);
    return true;
}

bool FileManager::deleteFile(std::string filename )
{
    cocos2d::JniMethodInfo minfo;
    bool ok = cocos2d::JniHelper::getStaticMethodInfo(minfo, CLASS_NAME, "deleteFile2", "(Ljava/lang/String;)Z");
    if ( !ok ) {
        CCLOG("Could not find instance method deleteFile2");
        return false;
    }
    jstring jstr = minfo.env->NewStringUTF( filename.c_str() );
    minfo.env->CallStaticBooleanMethod(minfo.classID, minfo.methodID, jstr);
    return true;
}

#endif
