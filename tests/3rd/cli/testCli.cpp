#include "cli/cli.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

int main(int argc, char** argv) {
    CLI::App app{"全面CLI11命令行Demo"};

    // 1. 位置参数（必选）
    std::string input_file;
    app.add_option("-i,--input", input_file, "输入文件")
        ->required()
        ->check(CLI::ExistingFile); // 校验文件存在

    // 2. 选项参数（可选，带默认值）
    std::string output_file;
    app.add_option("-o,--output", output_file, "输出文件")->default_val("result.txt");

    // 3. 标志参数（布尔类型）
    bool verbose = false;
    app.add_flag("-v,--verbose", verbose, "详细输出");

    // 4. 多值参数
    std::vector<int> numbers;
    app.add_option("-n,--number", numbers, "一组数字")
        ->expected(-1); // 任意数量

    // 5. 带范围校验的选项
    int level = 1;
    app.add_option("-l,--level", level, "处理级别 (1-5)")
        ->check(CLI::Range(1, 5))
        ->default_val(1);

    // 6. 子命令
    auto* subcmd = app.add_subcommand("analyze", "分析模式");
    std::string mode;
    subcmd->add_option("-m,--mode", mode, "分析方式")->required();

    // 7. 自动帮助和版本信息
    app.set_version_flag("-V,--version", "1.0.0");

    // 8. 解析命令行参数
    CLI11_PARSE(app, argc, argv);

    // 9. 打印解析结果
    std::cout << "输入文件: " << input_file << std::endl;
    std::cout << "输出文件: " << output_file << std::endl;
    std::cout << "详细模式: " << (verbose ? "是" : "否") << std::endl;
    std::cout << "级别: " << level << std::endl;

    if (!numbers.empty()) {
        std::cout << "数字: ";
        for (auto n : numbers) std::cout << n << " ";
        std::cout << std::endl;
    }

    if (subcmd->parsed()) {
        std::cout << "[分析模式] 方式: " << mode << std::endl;
    }

    // 10. 业务逻辑
    // ...此处可根据参数实现你的业务...

    return 0;
}

/*
./build/bin/cli -o CMakeLists.txt -v -n 12 3 4 -i CMakeLists.txt analyze -m mymode
输入文件: CMakeLists.txt
输出文件: CMakeLists.txt
详细模式: 是
级别: 1
数字: 12 3 4 
[分析模式] 方式: mymode
*/
