## 开发工具

### Pycharm

官网下载安装包后，按下列方式安装并启动

```shell
tar -xvzf pycharm-community-*tar.gz
cd pycharm-community*/bin
sh pycharm.sh
```

对 Ubuntu 18，固定图标到任务栏没有 Ubuntu 16 方便，如果要把图标固定在任务栏，则：

```
cd ~/.local/share/applications/
sudo gedit PyCharm.desktop
```

在 Pycharm.desktop 里填入以下内容：

```shell
[Desktop Entry]
Version=1.0
Terminal=false
Type=Application
Name=PyCharm
Exec=/home/user/soft/pycharm-community-2020.2.1/bin/pycharm.sh
Icon=/home/user/soft/pycharm-community-2020.2.1/bin/pycharm.png
NoDisplay=false
StartupWMClass=jetbrains-pycharm-ce
```

其中`Exec`表示安装软件的启动文件路径，`Icon` 表示安装软件的图标路径。保存后，右键点击刚生成的应用图标，选择添加到收藏夹，就会固定在任务栏。但双击任务栏的图标会出现双图标的现象，解决办法，终端输入`xprop |grep WM_CLASS`此时鼠标会变成十字准心，然后点击一下正在运行的软件，终端会反馈一条信息，包括2个字符串。显示如下：

```shell
$ xprop |grep WM_CLASS
WM_CLASS(STRING) = "jetbrains-pycharm-ce", "jetbrains-pycharm-ce"
```

将输出的字符串填入`PyCharm.desktop`的`StartupWMClass`变量。

参考[Ubuntu18.04软件图标（PyCharm）固定菜单栏方法](https://blog.csdn.net/ayue1220/article/details/90701463)

### Android Studio

SDK和Android studio都安装在`/usr/local/` 下，分别建文件夹 `java/jdk_14.0.2`和`java/android-studio`

```shell
$ cd /usr/local/
$ mkdir java
$ cd java
$ mkdir jdk_14.0.2
$ mkdir android-studio
```

1、Sdk 安装，[点击下载](https://www.oracle.com/java/technologies/javase-jdk14-downloads.html)

```shell
# 解压 JDK
sudo tar -zxvf jdk-14.0.2_linux-x64_bin.tar.gz
# 在jdk文件夹中输入以下命令生成jre
sudo bin/jlink --module-path jmods --add-modules java.desktop --output jre
```

2、Android Studio安装，[点击下载](https://developer.android.google.cn/studio/)
bin文件夹下：./studio.sh
除了要 costom，其他默认。
然后第一次启动，安装一堆工具包。有问题一定是网络的问题，首先考虑换热点。

环境：

```shell
sudo gedit ~/.bashrc
```

在末尾填入：

```shell
# java
export JAVA_HOME=/usr/local/java/jdk_14.0.2
export JRE_HOME=${JAVA_HOME}/jre
export CLASSPATH=.:{JAVA_HOME}/lib:${JRE_HOME}/lib
export PATH=${JAVA_HOME}/bin:$PATH

# android studio
export ANDROID_HOME=/usr/local/java/android-stodio/bin
export PATH=$PATH:$ANDROID_HOME

# android SDK
export ANDROID_SDK_HOME=/home/user/Android/Sdk
export PATH=$PATH:$ANDROID_SDK_HOME/platform-tools
```

最后激活配置：

```shell
source   ~/.bashrc
```

固定图标到任务栏

```shell
cd ~/.local/share/applications/
sudo gedit AndroidStudio.desktop
```

在 AndroidStudio.desktop 里填入以下内容：

```shell
[Desktop Entry]
Version=1.0
Terminal=false
Type=Application
Name=Android Studio
Exec=/usr/local/java/android-studio/bin/studio.sh
Icon=/usr/local/java/android-studio/bin/studio.png
NoDisplay=false
StartupWMClass=jetbrains-studio
```

这时双击任务栏的图标会出现双图标的现象，解决办法，终端输入`xprop |grep WM_CLASS`此时鼠标会变成十字准心，然后点击一下正在运行的软件，终端会反馈一条信息，包括2个字符串。显示如下：

```shell
$ xprop |grep WM_CLASS
WM_CLASS(STRING) = "sun-awt-X11-XFramePeer", "jetbrains-studio"
```

将输出的字符串`jetbrains-studio`填入`PyCharm.desktop`的`StartupWMClass`变量。实际上把`sun-awt-X11-XFramePeer`填入`StartupWMClass`变量也没问题。

## 配置

### [中文 man](https://blog.csdn.net/BobYuan888/article/details/88865219)

```shell
sudo apt-get update                              # 更新你的下载源目录，此步骤可省略。
sudo apt-get install manpages-zh   # 系统会自动下载并安装
sudo gedit ~/.bashrc                             # 在最后一行输入：alias cman='man -M /usr/share/man/zh_CN'  
                                                                       # 将中文的man命令重命名为cman命令，之后保存并退出编辑
source ~/.bashrc                                     # 重新运行.bashrc文件
```

## 代码跟踪

### sourcetrail

[下载](https://github.com/CoatiSoftware/Sourcetrail/releases)

解压后运行`install.sh`脚本

### doxygen

```shell
sudo apt-get install doxygen
sudo apt-get install graphviz
```

**Usage**

获取[配置文件](./src/Doxyfile)

```shell
$ doxygen -g
# 此时用获取的配置文件覆盖掉新创建的配置文件
$ doxygen 
```

**参考**

- [Linux 下 Doxygen 安装及使用大全](https://blog.csdn.net/t990423909/article/details/106169890)
- [配置文件设置](https://www.cnblogs.com/Mr--Yang/p/6338095.html)

## 常用软件

### teamviewer

```
$ sudo dpkg -i teamviewer*.deb
```

不出意外会出现一些错误，需要安装一些依赖。使用下面的修复依赖关系的命令：

```
$ sudo apt-get install -f
```

再次执行命令:

```
$ sudo dpkg -i teamviewer*.deb
```

### 百度网盘

[下载 .deb 格式的软件包](https://pan.baidu.com/download)

```shell
$ sudo dpkg -i baidunetdisk_3.4.1_amd64.deb
```

### 微信

[下载微信安装包](https://www.cnblogs.com/limanjihe/p/9963574.html)

链接: https://pan.baidu.com/s/1YIswiB6UnTPsVD2K162j8w  密码: 0wtt

```shell
# 解压后
$ electronic-wechat
# 如果不能启动，则
$ sudo snap install electronic-wechat
$ electronic-wechat
```

## 娱乐软件

### 网易云

```
sudo dpkg -i netease-cloud-music*.deb
```

显示安装错误，因为缺少依赖,使用如下命令安装相关依赖：

```
sudo apt-get install -f 
```

再重复安装命令:

```
sudo dpkg -i netease-cloud-music*.deb
```

启动客户端：

```
netease-cloud-music
```

参考链接： [网易云安装](https://blog.csdn.net/zz531987464/article/details/83050067)

