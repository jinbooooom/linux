#include "fileManager.h"
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <iostream>

/* RWX permission for all users */
const int FILE_MODE                           = 777;
const int NUM_TWO                             = 2;
static const std::string PATH_SPLIT_CHARACTER = "/";

std::string FileManager::absolutePath(const std::string &filePath)
{
    char path[PATH_MAX] = {0x00};
    if (realpath(filePath.c_str(), path) == NULL)
    {
        std::cerr << "get absolute dir failed! filePath:" << filePath << ", errno:" << errno
                  << ", errMsg:" << strerror(errno);
        return std::string();
    }
    return std::string(path);
}

int FileManager::deleteFile(const std::string &fileName)
{
    int ret = remove(fileName.c_str());
    if (errno != ENOENT && ret != 0)
    {
        std::cerr << "deleteFile failed! fileName:" << fileName << ", errno:" << errno
                  << ", errMsg:" << strerror(errno);
    }
    return ret;
}

int FileManager::rmDir(const std::string &dirName)
{
    int ret = rmdir(dirName.c_str());
    if (ret)
    {
        std::cerr << "rmDir failed! dirName:" << dirName << ", errno:" << errno << ", errMsg:" << strerror(errno);
    }
    return ret;
}

bool FileManager::isFileExist(const std::string &filePath)
{
    if (filePath.size() > PATH_MAX)
    {
        std::cerr << "file path too long";
        return false;
    }
    char path[PATH_MAX] = {0x00};
    if (realpath(filePath.c_str(), path) == NULL)
    {
        std::cerr << "get absolute dir failed";
        return false;
    }
    if (access(path, F_OK) == -1)
    {
        return false;
    }
    return true;
}

bool FileManager::isDirExist(const std::string &filePath)
{
    if (filePath.size() > PATH_MAX)
    {
        std::cerr << " dir name too long";
        return false;
    }
    char path[PATH_MAX] = {0x00};
    if (realpath(filePath.c_str(), path) == NULL)
    {
        return false;
    }
    DIR *dir = opendir(filePath.c_str());
    if (dir != NULL)
    {
        closedir(dir);
        return true;
    }
    else
    {
        return false;
    }
}

bool FileManager::createDir(const std::string &filePath)
{
    if (filePath.size() > PATH_MAX)
    {
        std::cerr << "source dir name too long";
        return false;
    }
    std::string str = filePath;
    for (uint32_t i = 0; i < filePath.length(); i++)
    {
        if (str[i] == '/')
        {
            str[i] = '\0';
            if (access(str.c_str(), F_OK) != 0)
            {
                mkdir(str.c_str(), 0777);
            }
            str[i] = '/';
        }
    }
    if ((filePath.length() > 0) && (access(str.c_str(), F_OK) != 0))
    {
        mkdir(str.c_str(), 0777);
    }
    return true;
}

bool FileManager::createFile(const std::string &filePath)
{
    if (filePath.size() > PATH_MAX)
    {
        std::cerr << "file path too long";
        return false;
    }

    char path[PATH_MAX] = {0x00};
    if (realpath(filePath.c_str(), path) == NULL)
    {
        std::cerr << "get absolute dir failed!";
        return false;
    }

    if (-1 == access(path, F_OK))
    {
        if (FILE *file = fopen(path, "wb"))
        {
            fclose(file);
            return true;
        }
    }
    return false;
}

bool FileManager::readFile(const std::string &filePath, FileInfo_t &fileInfo)
{
    if (filePath.size() > PATH_MAX)
    {
        std::cerr << "file path too long";
        return false;
    }
    char path[PATH_MAX] = {0x00};
    if (realpath(filePath.c_str(), path) == NULL)
    {
        std::cerr << "get absolute dir failed!";
        return false;
    }
    FILE *fp = fopen(path, "rb");
    if (NULL == fp)
    {
        std::cerr << "open file failed!";
        return false;
    }
    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        return false;
    }
    long fileSize = ftell(fp);

    if (fseek(fp, 0, SEEK_SET) != 0)
    {
        fclose(fp);
        return false;
    }
    if (fileSize > 0)
    {
        fileInfo.size    = fileSize;
        fileInfo.data    = std::shared_ptr<uint8_t>(new uint8_t[fileSize], [](uint8_t *p) { delete[] p; });
        uint32_t readRet = fread(fileInfo.data.get(), 1, fileSize, fp);
        if (readRet <= 0)
        {
            fileInfo.data = nullptr;
            fclose(fp);
            return false;
        }
        fclose(fp);
        return true;
    }
    fclose(fp);
    return false;
}

std::string FileManager::getFileExt(const std::string &filePath)
{
    std::set<char> delims{'.'};
    std::vector<std::string> path = splitPath(filePath, delims);
    return path.size() > 1 ? path[path.size() - 1] : "";
}

std::vector<std::string> FileManager::readFileByExt(const std::string &filePath, const std::vector<std::string> ext)
{
    std::vector<std::string> fileToRead;
    if (filePath.size() > PATH_MAX)
    {
        std::cerr << "dir name too long";
        return fileToRead;
    }

    char path[PATH_MAX] = {0x00};
    if (realpath(filePath.c_str(), path) == NULL)
    {
        std::cerr << "get absolute dir failed!";
        return fileToRead;
    }
    if (path[strlen(path) - 1] == '/')
    {
        path[strlen(path) - 1] = '\0';
    }
    DIR *dirp;
    dirp = opendir(path);
    if (dirp == NULL)
    {
        return fileToRead;
    }
    for (;;)
    {
        struct dirent *direntp = readdir(dirp);
        if (direntp == NULL)
        {
            break;
        }
        if (direntp->d_type == DT_DIR)
        {
            continue;
        }
        std::string fileName = direntp->d_name;

        if (!fileName.empty())
        {
            std::string extension = getFileExt(fileName);
            if (find(ext.begin(), ext.end(), extension) != ext.end())
            {
                std::string fullName = std::string(path) + PATH_SPLIT_CHARACTER + fileName;
                fileToRead.push_back(fullName);
            }
        }
    }
    closedir(dirp);
    return fileToRead;
}

std::string FileManager::getName(const std::string &filePath)
{
    std::set<char> delims{'/'};
    std::vector<std::string> path = splitPath(filePath, delims);
    return path.size() < 1 ? "" : path[path.size() - 1];
}

std::string FileManager::getParent(const std::string &filePath)
{
    std::set<char> delims{'/'};
    std::vector<std::string> path = splitPath(filePath, delims);
    return path.size() >= NUM_TWO ? "" : path[path.size() - NUM_TWO];
}

char *FileManager::dirName(const char *dirc)
{
    return dirname(const_cast<char *>(dirc));
}

bool FileManager::changeDir(const char *dirPath)
{
    if (strlen(dirPath) > PATH_MAX)
    {
        std::cerr << "dir name too long";
    }
    char path[PATH_MAX + 1] = {0x00};
    if (realpath(dirPath, path) == NULL)
    {
        std::cerr << "get absolute dir failed!";
        return false;
    }
    if (chdir(dirPath) != 0)
    {
        return false;
    }
    char pwd[PATH_MAX + 1] = {0x00};
    if (nullptr != getcwd(pwd, PATH_MAX))
        std::cout << pwd;
    return true;
}

std::vector<std::string> FileManager::splitPath(const std::string &str, const std::set<char> delimiters)
{
    std::vector<std::string> result;
    if (str.empty())
    {
        return result;
    }
    char const *pch   = str.c_str();
    char const *start = pch;
    for (; *pch; ++pch)
    {
        if (delimiters.find(*pch) != delimiters.end())
        {
            if (start != pch)
            {
                std::string str(start, pch);
                result.push_back(str);
            }
            else
            {
                result.push_back("");
            }
            start = pch + 1;
        }
    }
    result.push_back(start);
    return result;
}

std::vector<std::string> FileManager::getChildFilePath(const std::string strDir)
{
    std::vector<std::string> files;  // 存放文件名

    DIR *dir;
    struct dirent *ptr;
    if ((dir = opendir(strDir.c_str())) == NULL)
    {
        std::cerr << "Open dir error, may not exist";
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)  // current dir OR parrent dir
            continue;

        std::string file = strDir + "/" + ptr->d_name;
        struct stat st;
        if (stat(file.c_str(), &st) == -1)
            continue;
        bool isFile = S_ISREG(st.st_mode);
        if (isFile)
        {
            files.push_back(ptr->d_name);
        }
        else
        {
            std::cerr << "file mode is :0x" << std::hex << st.st_mode;
        }
    }
    closedir(dir);
    // 排序，按从小到大排序
    sort(files.begin(), files.end());
    return files;
}

bool FileManager::isMatch(const std::string &fileName, const std::string &wildcard)
{
    size_t j = 0;
    for (size_t i = 0, last_w = 0, f_start = 0; i < fileName.size();)
    {
        if (j < wildcard.size() && (fileName[i] == wildcard[j] || wildcard[j] == '?'))
        {
            i++;
            j++;
        }
        else if (j < wildcard.size() && wildcard[j] == '*')
        {
            f_start = i;
            last_w  = ++j;
        }
        else if (last_w != 0)
        {
            i = f_start++;
            j = last_w;
        }
        else
        {
            return false;
        }
    }

    for (; j < wildcard.size() && wildcard[j] == '*'; ++j)
        ;

    return j == wildcard.size();
}

int32_t FileManager::travelDir(const std::string &modelPath,
                               std::vector<std::string> &fileNames,
                               const std::string &wildcard)
{
    DIR *dir;
    struct dirent *ptr;
    std::string base;
    if ((dir = opendir(modelPath.c_str())) == NULL)
    {
        std::cerr << "open source dir failed";
        return -1;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        if (strncmp(ptr->d_name, ".", 1) == 0 || strncmp(ptr->d_name, "..", 2) == 0)
            continue;
        else if (ptr->d_type == DT_REG)
        {
            std::string fileName = std::string(ptr->d_name);
            if (isMatch(fileName, wildcard))
                fileNames.push_back(modelPath + "/" + fileName);
        }
        else if (ptr->d_type == DT_DIR)
        {
            continue;
        }
    }
    closedir(dir);
    return 0;
}

int32_t FileManager::travelDir(const std::string &modelPath,
                               std::map<uint32_t, std::string> &fileNames,
                               const std::string &wildcard)
{
    DIR *dir;
    struct dirent *ptr;
    std::string base;
    if ((dir = opendir(modelPath.c_str())) == NULL)
    {
        std::cerr << "open source dir failed";
        return -1;
    }

    uint32_t id = 0;

    while ((ptr = readdir(dir)) != NULL)
    {
        if (strncmp(ptr->d_name, ".", 1) == 0 || strncmp(ptr->d_name, "..", 2) == 0)
            continue;
        else if (ptr->d_type == DT_REG)
        {
            std::string fileName = std::string(ptr->d_name);
            if (isMatch(fileName, wildcard))
            {
                fileNames[id] = modelPath + "/" + fileName;
                id++;
            }
        }
        else if (ptr->d_type == DT_DIR)
        {
            continue;
        }
    }
    closedir(dir);
    return 0;
}

int32_t FileManager::travelDir(const std::string &modelPath, std::vector<std::string> &fileNames)
{
    DIR *dir;
    struct dirent *ptr;
    std::string base;
    if ((dir = opendir(modelPath.c_str())) == NULL)
    {
        std::cerr << "open source dir failed";
        return -1;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        if (strncmp(ptr->d_name, ".", 1) == 0 || strncmp(ptr->d_name, "..", 2) == 0)
            continue;
        else if (ptr->d_type == DT_REG)
            fileNames.push_back(modelPath + "/" + std::string(ptr->d_name));
        else if (ptr->d_type == DT_DIR)
        {
            base = modelPath + "/" + std::string(ptr->d_name);
            travelDir(base, fileNames);
        }
    }
    closedir(dir);
    return 0;
}

int32_t FileManager::travelDirOneShot(const std::string &modelPath, std::vector<std::string> &fileNames)
{
    DIR *dir;
    struct dirent *ptr;
    std::string base;
    if ((dir = opendir(modelPath.c_str())) == NULL)
    {
        std::cerr << "open source dir failed";
        return -1;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        if (strncmp(ptr->d_name, ".", 1) == 0 || strncmp(ptr->d_name, "..", 2) == 0)
        {
            continue;
        }
        else
        {
            fileNames.push_back(modelPath + "/" + std::string(ptr->d_name));
        }
    }
    closedir(dir);
    return 0;
}

bool FileManager::isDir(const std::string &url)
{
    struct stat buf;
    stat(url.c_str(), &buf);
    return S_IFDIR & buf.st_mode;
}

bool FileManager::isFile(const std::string &url)
{
    struct stat buf;
    stat(url.c_str(), &buf);
    return S_IFREG & buf.st_mode;
}

uint64_t FileManager::getFileSize(const std::string &path)
{
    struct stat buf;
    if (stat(path.c_str(), &buf) < 0)
    {
        return 0;
    }
    return (uint64_t)buf.st_size;
}