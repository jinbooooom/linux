## Flatbuffers简介

### Flatbuffers由来

FlatBuffers为Google发布的一个跨平台，提供多种语言接口，注重性能和资源使用的序列化类库。目前该类库提供C++, C#, C, Go, Java, JavaScript, PHP, and Python语言接口。该序列化类库多用于移动端手游数据传输以及特定的对性能有较高要求的应用。

### Flatbuffers特点

- **不需要打包/解包。它的结构化数据都以二进制形式保存，不需要数据解析过程，数据也可以方便传递**

- 省内存、性能好。数据访问速度接近原生的struct，只多了一次解引用（根据起始地址和偏移量，然后取值）
- 弹性。都是可选字段，支持向前、向后兼容
- 强类型系统。在编译阶段就能预防一些bug的产生
- 易用。可以解析schema和json文本
- 跨平台（C++11/Java/Go/C#）

### Flatbuffers 与 Probobuf、JSON 的比较

- FlatBuffers的功能和Protocol Buffers很像，他们的最大不同点是在使用具体的数据之前，FlatBuffers不需要解析/解包的过程。
- 在工程中使用时，FlatBuffers的引用比Protocol Buffers方便很多，只需要包含两三个头文件即可
- JSON作为数据交换格式，被广泛用户各种动态语言之间（当然也包括静态语言）。它的优点是易于理解（可读性好），同时它的最大的缺点那就是解析时的性能问题。而且因为它的动态类型特点，代码可能还需要多写好多类型、数据检查逻辑。

## 序列化

### 什么是数据序列化

序列化 (Serialization)是将对象的状态信息转换为可以存储或传输的形式的过程。在序列化期间，对象将其当前状态写入到临时或持久性存储区。以后，可以通过从存储区中读取或反序列化对象的状态，重新创建该对象。

### 为什么要做序列化

序列化的原因基本三种情况： 

- 永久性保存对象，保存对象的字节序列到本地文件中；
- 对象在网络中传递； 
- 对象在IPC间传递。

在XML、JSON、Protobuf和Flatbuffers这几个序列化工具中，Flatbuffers在数据序列化方面性能最佳，而且使用起来只需要包含几个头文件，不依赖第三方动态链接库。

## Flatbuffers获取和安装

Flatbuffers官方源码地址：

https://github.com/google/flatbuffers.git，该工程使用的版本是release1.11.0

```shell
cd flatbuffers-1.11.0/
# 执行cmake，选择安装目录，如果不指定会安装至系统目录
cmake -DCMAKE_INSTALL_PREFIX:PATH=/home/path/to/flatbuffers -G “Unix Makefiles”
# 执行源码编译
make
# 编译完成之后安装
make install
```

安装完成之后会在安装目录下形成flatbuffers目录，子目录中存在：

- --bin/，编译产生的二进制可执行文件flatc，它用来根据fbs文件生成头文件
- --lib/，编译产生的静态链接库libflatbuffers.a，flatbuffers里某些功能函数的实现需要链接该库
- --include/，flatbuffers所有头文件，正常使用时，只需要包含这些头文件，可以不链接上文中的静态库，用户应用程序即可编译通过

## Flatbuffers使用

数据序列化技术一般包含如下组件：

- IDL（Interface description language）文件，参与通讯的各方需要对通讯的内容需要做相关的约定，为了建立一个与语言和平台无关的约定，这个约定需要采用与具体开发语言、平台无关的语言来进行描述。这种语言被称为接口描述语言（IDL），采用IDL撰写的协议约定称之为IDL文件
- IDL Compiler，IDL文件中约定的内容为了在各语言和平台可见，需要有一个编译器，将IDL文件转换成各语言对应的动态库
- Stub/Skeleton Lib，负责序列化和反序列化的工作代码
- Client/Server，应用层程序代码，他们面对的是IDL所生成的特定语言的class或struct
- 底层协议栈和互联网、IPC传输通道，，序列化之后的数据通过底层的传输层、物理层协议在互联网或者IPC传输通道中传输

FlatBuffers 是一个高效的数据格式，但要实现效率，需要一个高效的 schema（对应序列化组件中的IDL）。使用FlatBuffers的第一步就是编写Schema文件，它支持多种数据类型。字段可以有标量类型（所有大小的整数/浮点数），也可以是字符串，任何类型的数组，引用另一个对象，或者一组可能的对象（Union）。

### Table vs Struct

- Table 中每个字段都是可选 optional 的，并会设置默认值为0/null
- Table 允许新添加字段，但只能在定义的末尾添加，具有后向兼容性
- Table 不允许删除不再使用的字段，但可以通过把它标记为 deprecated，从而停止将它们写入数据，这与删除的效果相同
- Table 允许更改字段，但只有在类型改变是相同大小的情况下，是可行的
- structs 没有任何字段是可选的（所以也没有默认值）
- structs 字段可能不会被添加或被弃用
- structs 可能只包含标量或其他结构
- structs 不提供前向/后向兼容性，但占用内存更小。对于不太可能改变的非常小的对象（例如坐标对或RGBA颜色）存成 struct 是非常有用的

table 的内存开销很小（因为 vtables 很小并且共享）访问成本也很小（间接访问），但是提供了很大的灵活性。table 甚至可能比等价的 struct 花费更少的内存，因为字段在等于默认值时不需要存储在 buffer 中。

如果确定以后不会进行任何更改，structs 使用的内存少于 table，并且访问速度更快（它们总是以串联方式存储在其父对象中，并且不使用虚拟表）。

### 关于类型的说明

- 一旦一个类型声明了，尽量不要改变它的类型，一旦改变了，很可能就会出现错误。例如如果把 int 改成 uint，数据如果有负数，那么就会出错。
- 标量类型的字段有默认值，非标量的字段(string/vector/table)如果没有值的话，默认值为 NULL。
- 只应添加枚举值，不要去删除枚举值（对枚举不存在弃用一说）。
- Unions 字段，该字段可以包含对这些类型中的任何一个的引用，即这块内存区域只能由其中一种类型使用。union 跟 enum 比较类似，但是 union 包含的是 table，enum 包含的是 scalar或者 struct。
- Root type 声明了序列化数据的根表（或结构）。这对于解析不包含对象类型信息的 JSON 数据尤为重要。可以在一个Schema文件中定义多个Table，而只定义一个root_type，使用 flatbuffers::GetRoot(bufp); 访问非root_type指定的Table。
- table 在处理 field 数量非常多，但是实际使用只有其中少数几个 field 这种情况，效率依旧非常高。因此，组织数据应该尽可能的组织成 table 的形式。
- 如果可能的话，尽量使用枚举的形式代替字符串。
- FlatBuffers 默认可以支持存放的下所有整数，因此尽量选择所需的最小大小，而不是默认为 int/long。

### 关于命名

- Table, struct, enum and rpc names (types) 采用大写驼峰命名法。
- Table and struct field names 采用下划线命名法。这样做方法自动生成小写驼峰命名的代码。
- Enum values 采用大写驼峰命名法。
- namespaces 采用大写驼峰命名法。
- 大括号：与声明的开头位于同一行。
- **间距：缩进2个空格。:两边没有空格，=两边各一个空格**。

### Schema文件示例

Schema文件以.fbs结尾，示例如下：

namespace TestApp;

struct KV {

 key: ulong;

 value: double;

}

table TestObj {

 id:ulong;

 name:string;

 flag:ubyte = 0;

 list:[ulong];

 kv:KV;

}

root_type TestObj;

## 编译schema文件

将上节编写的scheme文件保存为test.fbs文件，到该文件所在文件夹下，运用编译产生的flatc（对应序列化组件中的编译器），执行：

```shell
flatc [ GENERATOR OPTIONS ] [ -o PATH ] [ -I PATH ] [ -S ] FILES… [ – FILES…]
```

定义文件按照顺序被读取和编译,还可以包含其他定义文件和数据(详情见下面)。

任意个定义文件可能生成一个或者多个定义文件，这取决于附加的命令选项

- --cpp,-c :按照定义生成C++头文件
- --java,-j :按照定义生成Java代码
- --csharp,-n :按照定义生成C#代码
- --go,-g :按照定义生成Go代码
- --python,-p :按照定义生成python打底吗
- --javascript,-s :按照定义生成JavaScript
- --php :按照定义生成php

数据序列化格式选项

- --binary,-b :序列化成.bin 后缀的二进制格式,
- --json,-t :序列化成.json 后缀的json格式,

其他选项

- -o PATH :输出搜有生成的文件到Path(绝对路径,或者相对于当前目录)路径,如果省略,Path就是当前目录.路径末尾因该是你的系统分隔符\或者/.
- -I PATH :当遇见include声明,试图读取文件的时候将从此路径按照顺序查找,如果失败,就按照相对路径查找
- -M :打印Make规则到生成文件

- --strict-json :要求生成严格的json文件(名字等字段包含在引号中,table和Vector末尾没有逗号),默认 在required/generated时没有引号,末尾逗号是允许的
- ---defaults-json :当输出json文件本的时候输出字段等于默认值
- ---no-prefix :当生成C++头文件时 枚举值不包含枚举类型的前缀
- ---scoped-enums :使用C++11风格作用域和强类型枚举生成C++,也就意味着 --no-prefix
- ---no-includes :不生成包含include模式的代码,(依赖C++)
- ---gen-mutable :为可变的FlatBuffers生成额外的non-const访问器
- ---gen-onefile :生成一个定义文件(用于C#)

由于我主要使用C++，所以在编译时只需要执行

```shell
./bin/flatc -c -b  test.fbs
```

即可在当前目录下产生对应的`test_generated.h`头文件

## 序列化与反序列化

主程序请参考当前目录下的文件`main.cpp`

### 程序编译

```shell
 g++ test.cpp -Iinclude -o test -std=c++14
```

