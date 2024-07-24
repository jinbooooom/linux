# -*- coding: utf-8 -*-
"""
Author: Jinbo
Description: This script formats files in a specified directory using clang-format.
"""

import os
import argparse

def format_files(directory, clang_format_file, exclude_dirs):
    # 检查 clang-format 文件是否存在
    if not os.path.isfile(clang_format_file):
        print(f"未找到 clang_format 文件: {clang_format_file}")
        return
    
    # 设置要格式化的文件扩展名
    file_extensions = ['.c', '.cpp', '.h', '.hpp']

    # 存储要格式化的文件路径
    files_to_format = []

    # 搜索指定路径下的所有文件
    for root, dirs, files in os.walk(directory):
        for exclude_dir in exclude_dirs:
            if exclude_dir in dirs:
                dirs.remove(exclude_dir)

        for file in files:
            # 检查文件扩展名是否在目标扩展名列表中
            if file.endswith(tuple(file_extensions)):
                file_path = os.path.join(root, file)
                files_to_format.append(file_path)

    # 格式化所有文件
    if files_to_format:
        batch_size = 32  # 每组文件的大小
        num_batches = (len(files_to_format) + batch_size - 1) // batch_size  # 计算需要的批次数

        for i in range(num_batches):
            start_idx = i * batch_size
            end_idx = min((i + 1) * batch_size, len(files_to_format))
            batch_files = files_to_format[start_idx:end_idx]
            formatted_files = " ".join(batch_files)
            os.system(f"clang-format -i -style=file {formatted_files}")

            progress = ((i + 1) / num_batches) * 100
            print(f"clang-format {progress:.2f}%")

            # print(f"已格式化以下文件 (批次 {i+1} / {num_batches})")
            # for file_path in batch_files:
            #     print(file_path)

def main():
    default_exclude_dirs = ["3rd",
                            "assets",
                            "communication",
                            "document",
                            "tests/3rd/flatbuffers/",
                            "tools",
                            "build",
                            ]

    # 解析命令行参数
    parser = argparse.ArgumentParser(description="格式化指定目录下的文件")
    parser.add_argument("-d", "--dir", default="./", help="要格式化的目录路径")
    parser.add_argument("-f", "--format", default=".clang-format", help="使用的 clang-format 路径")
    parser.add_argument("-e", "--exclude", nargs='+', default=default_exclude_dirs, help="需要排除的目录")
    args = parser.parse_args()

    # 调用格式化函数
    format_files(args.dir, args.format, args.exclude)

if __name__ == "__main__":
    main()
