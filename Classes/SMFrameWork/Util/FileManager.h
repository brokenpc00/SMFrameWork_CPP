//
//  FileManager.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 17..
//
//

#ifndef FileManager_h
#define FileManager_h

#include <cocos2d.h>
#include <string>
#include <mutex>
#include <vector>

class FileManager {
public:
    static const int SUCCESS;
    static const int FAILED;
    
    enum class FileType
    {
        Image,
        Doc,
        SnapShot,
        ZIP,
        XML,
        Preload,
        DB,
        EPUB_DOWN,
        EPUB_EXTRACT
    };
    
    static FileManager* getInstance();
    
    
protected:
    FileManager();
    virtual ~FileManager();
    
    bool init();
    
public:
    std::string getFullFilePath(const FileType type, const std::string& fileName);
    std::string getLocalFilePath(const FileType type, const std::string& fileName);
    
    std::string getFullPath(const FileType type);
    std::string getLocalPath(const FileType type);
    
    bool isFileEixst(const FileType type, const std::string& fileName);
    bool writeToFile(const FileType type, const std::string& fileName, u_char* buffer, size_t bufSize);
    
    cocos2d::Data loadFromFile(const FileType type, const std::string& fileName, int *error);   // 0->success, -1->failed
    std::string loadStringFromFile(const FileType type, const std::string& fileName, int *error);   // 0->success, -1->failed
    
    bool removeFile(const FileType type, const std::string& fileName);
    bool renameFile(const FileType type, const std::string& oldName, std::string& newName);
    void clearCache(const FileType type);
    
    std::string createSaveFileName(const std::string& prefix="", const std::string& postfilx="");
    
    std::string getFilePath(const std::string fullPath);
    std::string getFileName(const std::string fullPath);

    // director access
    bool copyFile(std::string src, std::string dst);
    bool deleteFile(std::string file);
    bool createFolder(std::string folder);
    int listFolder(std::string folder, std::vector< std::string >& entries, bool directoriesOnly);
    
    
private:
    std::mutex _mutex;
    std::vector<std::string> getFileList(const FileType type);
};


#endif /* FileManager_h */
