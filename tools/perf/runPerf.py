#!/usr/bin/env python3

import sys
import argparse
import subprocess

# 设置命令行参数解析器
parser = argparse.ArgumentParser()


parser.add_argument("-e", "--extern", type=str, default="", help="extern string")

args = parser.parse_args()


# 执行命令行
externCmd = args.extern
    
# sudo perf record -F 99 -e cycles -g ./program
# sudo perf script > perf.data.txt -f
# ./FlameGraph/stackcollapse-perf.pl perf.data.txt | ./FlameGraph/flamegraph.pl > flamegraph.svg

perfRecordCommand = f"sudo perf record -F 99 -e cycles -g {externCmd}"
perfScriptCommand = f"sudo perf script > perf.data.txt -f"
flameGraphCommand = f"./FlameGraph/stackcollapse-perf.pl perf.data.txt | ./FlameGraph/flamegraph.pl > flamegraph.svg"

print(perfRecordCommand)
subprocess.run(perfRecordCommand, shell=True)
print(perfScriptCommand)
subprocess.run(perfScriptCommand, shell=True)
print(flameGraphCommand)
subprocess.run(flameGraphCommand, shell=True)


