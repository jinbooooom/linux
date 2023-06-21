cmake_minimum_required(VERSION 3.10)

execute_process(
    COMMAND bash "-c" "git log|head -n 1 |awk '{printf $2}'"
    OUTPUT_VARIABLE GIT_COMMIT
)

add_definitions(-DGIT_COMMIT_SHA1="${GIT_COMMIT}")
add_executable(testShowGitInfo testShowGitInfo.cpp)
