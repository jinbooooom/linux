#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

/**
 * @brief file information struct
 */
typedef struct FileInfo_t
{
    uint32_t size;                  ///< file size
    std::shared_ptr<uint8_t> data;  ///< file content
};

/**
 * @brief file manager class
 */
class FileManager
{
  public:
    /**
     * @brief get absolute path of the file
     * @param[in] path file path
     * @return std::string real absolute path
     */
    static std::string absolutePath(const std::string &filePath);

    /**
     * @brief delete file
     * @param[in] fileName
     * @return  success return 0, otherwise -1
     */
    static int deleteFile(const std::string &fileName);

    /**
     * @brief  remove a empty dir
     * @param[in] dirName
     * @return  success return 0, otherwise -1
     */
    static int rmDir(const std::string &dirName);

    /**
     * @brief judge whether the file exist
     * @param[in] path file path
     * @return bool true-exist\n
     * false-not exist
     */
    static bool isFileExist(const std::string &path);

    /**
     * @brief judge whether the directory exist
     * @param[in] path directory path
     * @return bool true-exist\n
     * false-not exist
     */
    static bool isDirExist(const std::string &path);

    /**
     * @brief create a directory
     * @param[in] path directory path
     * @return bool true-success\n
     * false-failed
     */
    static bool createDir(const std::string &path);

    /**
     * @brief create a normal file
     * @param[in] path file path
     * @return bool true-success\n
     * false-failed
     */
    static bool createFile(const std::string &path);

    /**
     * @brief read a normal file
     * @param[in] path file path
     * @param[out] fileInfo file information
     * @return bool true-success\n
     * false-failed
     */
    static bool readFile(const std::string &path, FileInfo_t &fileInfo);

    /**
     * @brief read file by extension
     * @param[in] path file path
     * @param[in] ext file extension
     * @return std::vector<std::string>
     */
    static std::vector<std::string> readFileByExt(const std::string &path, const std::vector<std::string> ext);

    /**
     * @brief get file extension
     * @param[in] path file path
     * @return std::string
     */
    static std::string getFileExt(const std::string &path);

    /**
     * @brief get name
     * @param[in] path file path or directory
     * @return std::string
     */
    static std::string getName(const std::string &path);

    /**
     * @brief get parent directory
     * @param[in] path directory
     * @return std::string
     */
    static std::string getParent(const std::string &path);

    /**
     * @brief get the directory name
     * @param[in] dirc full path
     * @return char*
     */
    static char *dirName(const char *dirc);

    /**
     * @brief change the current work directory
     * @param[in] path the target directory
     * @return bool true-success\n
     * false-failed
     */
    static bool changeDir(const char *path);

    /**
     * @brief split the current path
     * @param[in] str the input directory
     * @param[in] delimiters directory splitting characters
     * @return std::vector<std::string>
     */
    static std::vector<std::string> splitPath(const std::string &str, const std::set<char> delimiters);

    /**
     * @brief get the filepath of one dir
     * @param[in] strDir the file dir
     * @return std::vector<std::string> child file path names
     */
    static std::vector<std::string> getChildFilePath(const std::string strDir);
    /**
     * @brief get the file names of specific path includint its absolute directory info
     * @param[in] path the directory will be travelled
     * @param[io] fileNames filenames vector, incluing name and absolute directory
     * @return int32_t \n
     * - 0, Success
     * - -1, Failed
     */
    static int32_t travelDir(const std::string &path, std::vector<std::string> &fileNames);
    /**
     * @brief confirm whether the fileName matches the wildcard string
     *
     * @param[in]  fileName
     * @param[in]  wildcard
     * @return true
     * @return false
     */
    static bool isMatch(const std::string &fileName, const std::string &wildcard);
    /**
     * @brief get the file names of specific path including its absolute directory which matches the wildcard
     *
     * @param[in]  path the file path
     * @param[io]  fileNames the file names with matches the wildcard
     * @param[in]  wildcard
     * @return int32_t \n
     * - 0, Success
     * - 1, Failed
     */
    static int32_t travelDir(const std::string &path, std::vector<std::string> &fileNames, const std::string &wildcard);
    /**
     * @brief get the file names of specific path including its absolute directory which matches the wildcard
     * and save them in a map
     *
     * @param[in]  path the file path
     * @param[out]  fileNames the file names container with matches the wildcard
     * @param[in]  wildcard
     * @return int32_t \n
     * - 0, Success
     * - 1, Failed
     */
    static int32_t travelDir(const std::string &path,
                             std::map<uint32_t, std::string> &fileNames,
                             const std::string &wildcard);
    /**
     * @brief get child absolute directory info, if child path is dir, don't step in
     * @param[in] path the directory will be travelled
     * @param[io] fileNames filenames vector, incluing name and absolute directory
     * @return int32_t \n
     * - 0, Success
     * - -1, Failed
     */
    static int32_t travelDirOneShot(const std::string &modelPath, std::vector<std::string> &fileNames);
    /**
     * @brief judge the input url is directory or file
     *
     * @param  url input url
     * @return true
     * @return false
     */
    static bool isDir(const std::string &url);
    /**
     * @brief judge the input url is director or file
     *
     * @param  std::string
     * @return true
     * @return false
     */
    static bool isFile(const std::string &url);

    static uint64_t getFileSize(const std::string &path);
};
#endif  // __FILE_MANAGER_H__
