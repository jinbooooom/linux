## perf 工具权限问题

默认情况下，perf 工具需要特权（CAP_SYS_ADMIN）才能收集性能统计数据。

无权限类似如下的报错：

```shell
perf_event_open(..., PERF_FLAG_FD_CLOEXEC) failed with unexpected error 13 (Permission denied)
perf_event_open(..., 0) failed unexpectedly with error 13 (Permission denied)
Error:
You may not have permission to collect stats.

Consider tweaking /proc/sys/kernel/perf_event_paranoid,
which controls use of the performance events system by
unprivileged users (without CAP_SYS_ADMIN).

The current value is 3:

  -1: Allow use of (almost) all events by all users
      Ignore mlock limit after perf_event_mlock_kb without CAP_IPC_LOCK
>= 0: Disallow ftrace function tracepoint by users without CAP_SYS_ADMIN
      Disallow raw tracepoint access by users without CAP_SYS_ADMIN
>= 1: Disallow CPU event access by users without CAP_SYS_ADMIN
>= 2: Disallow kernel profiling by users without CAP_SYS_ADMIN

To make this setting permanent, edit /etc/sysctl.conf too, e.g.:

        kernel.perf_event_paranoid = -1

```

**调整 perf_event_paranoid 参数**：按照错误提示中的建议，您可以尝试调整 `/proc/sys/kernel/perf_event_paranoid` 参数来允许非特权用户收集性能统计数据。运行以下命令将其设置为 -1：

```shell
echo -1 | sudo tee /proc/sys/kernel/perf_event_paranoid
```

请注意，此操作可能需要管理员权限。

如果希望将更改保持永久生效，请编辑 `/etc/sysctl.conf` 文件，并添加或修改以下行：

```shell
kernel.perf_event_paranoid = -1
```

保存文件并重新加载 sysctl 配置：

```shell
sudo sysctl -p
```

这将在系统启动时应用更改。

## 火焰图

### 安装 FlameGraph

```
git clone https://github.com/brendangregg/FlameGraph.git
```

### 收集性能数据

使用 `perf record` 命令收集性能数据。以下是一个示例命令：

```shell
perf record -F 99 -e cycles -g -p <PID>
# 可以根据可执行程序名进行性能分析，而不使用 `-a` 参数。下面是一个示例命令
perf record -F 99 -e cycles -g ./your_program

sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/library perf record -F 99 -e cycles -g ./your_program [选项]
```

在上面的命令中，`-F 99` 表示采样频率为 99 Hz，并且使用 `-e cycles` 指定了要监测的事件（例如 CPU 周期计数）。`-g` 用于采集调用图（Callgraph）信息，`-p <PID>` 指定了要监测的进程 ID。将 `<PID>` 替换为要分析的实际进程的 PID。

通过在 `perf record` 命令之前设置 `LD_LIBRARY_PATH` 环境变量来指定动态库的路径。将 `/path/to/library` 替换为您实际动态库所在的路径。

这样，当 `perf` 工具运行 `./your_program` 时，它将使用处于指定路径下的动态库。

### 生成可读的文本文件

运行 `perf script` 命令将性能数据转换为可读的文本形式。

```shell
sudo perf script > perf.data.txt
# 若报错 File perf.data not owned by current user or root (use -f to override)
# 使用 sudo perf script > perf.data.txt -f
```

### 使用 FlameGraph 工具生成火焰图

运行以下命令将性能数据转换为火焰图。

```shell
./FlameGraph/stackcollapse-perf.pl perf.data.txt | ./FlameGraph/flamegraph.pl > flamegraph.svg
```

这将生成名为 `flamegraph.svg` 的火焰图文件。

使用浏览器打开生成的火焰图文件 `flamegraph.svg`，将能够可视化和分析程序的性能数据。火焰图展示了函数调用关系以及每个函数所占用的时间比例。



