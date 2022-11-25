## [下载和编译](https://valgrind.org/downloads/repository.html)

```shell
git clone https://sourceware.org/git/valgrind.git
cd valgrind
./autogen.sh
./configure --prefix=<installation-directory>  # installation-directory 只能是安装的文件夹的绝对路径
make
make install
# 然后检查使用 -g 选项编译的可执行程序，使用 -g 会显示检查的异常所在的行号
# ./vg-in-place path/to/execute.out
valgrind --tool=memcheck --track-fds=yes --leak-check=full --undef-value-errors=yes  --show-reachable=yes --log-file=log.txt path/to/a.out
```

参数说明：

- --tool=memcheck 指定Valgrind使用的工具。Valgrind是一个工具集，包括Memcheck、Cachegrind、Callgrind等多个工具。memcheck是缺省项。
- --track-fds 是否检查文件描述符。
- --leak-check 指定如何报告内存泄漏（memcheck能检查多种内存使用错误，内存泄漏是其中常见的一种），可选值有:
      no 不报告
      summary 显示简要信息，有多少个内存泄漏。summary是缺省值。
      yes 和 full 显示每个泄漏的内存在哪里分配。
- show-leak-kinds 指定显示内存泄漏的类型的组合。类型包括definite，indirect，possible，reachable。也可以指定all或none。缺省值是definite，possible。
- --log-file 报告文件名。如果没有指定，输出到stderr。

[内存问题分析工具_valgrind之memcheck基本使用]( https://blog.csdn.net/shyjhyp11/article/details/123575343)

## [一些示例](https://valgrind.org/docs/manual/mc-manual.html#mc-manual.leaks)

参考[pdf](https://valgrind.org/docs/manual/valgrind_manual.pdf)或者[网页](https://valgrind.org/docs/manual/mc-manual.html#mc-manual.leaks)章节4.2. Explanation of error messages from Memcheck

```shell
==105524== LEAK SUMMARY:
==105524==    definitely lost: 14 bytes in 2 blocks
==105524==    indirectly lost: 0 bytes in 0 blocks
==105524==      possibly lost: 0 bytes in 0 blocks
==105524==    still reachable: 0 bytes in 0 blocks
==105524==         suppressed: 0 bytes in 0 blocks

Definitely lost：确认丢失，程序中存在内存泄漏，需要尽快修复。
Indirectly lost：间接丢失，常与definitely lost一起出现，只要修复definitely lost即可。
Possibly lost：可能丢失，大多数情况下应视为definitely lost，需要尽快修复。
Still reachable：可以访问的，这些内存没有丢失、没有释放。建议修复。
Suppressed：已解决的，可以无视此部分。
```

