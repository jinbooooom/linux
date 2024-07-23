ipdb 是一款集成了Ipython的Python代码命令行调试工具，可以看做PDB的升级版

### 安装ipdb

```shell
pip install ipdb
```

### 调试方式

#### 集成式（侵入式）

在代码中设置 **ipdb.set_trace()**，然后再执行python代码，在**ipdb.set_trace()**的行会停下来，进入交互式调试模式。

```python3
a = 5
b = 6
import ipdb; ipdb.set_trace()  # 使用 pdb.set_trace() 设置断点，自动暂停
print(a+b)
```

#### 命令式（非侵入式）

集成式虽然方便，但是不够灵活，而且调试完后还要去掉**ipdb.set_trace()**代码，很麻烦。

可以用命令式调试方法，启动ipdb调试环境的方法为： 

```shell
python3  -m  ipdb  代码文件.py
```

## 常见命令

### h(help) 

h(help) - 帮助文档

```s
(Pdb) help

Documented commands (type help <topic>):
========================================
EOF    c          d        h         list      q        rv       undisplay
a      cl         debug    help      ll        quit     s        unt      
alias  clear      disable  ignore    longlist  r        source   until    
args   commands   display  interact  n         restart  step     up       
b      condition  down     j         next      return   tbreak   w        
break  cont       enable   jump      p         retval   u        whatis   
bt     continue   exit     l         pp        run      unalias  where    

Miscellaneous help topics:
==========================
exec  pdb

(Pdb) help bt
w(here)
        Print a stack trace, with the most recent frame at the bottom.
        An arrow indicates the "current frame", which determines the
        context of most commands.  'bt' is an alias for this command.
(Pdb) 
```

### l(list) 

l(list) 开始行,结束行 - 查看指定行数之间的代码（逗号很重要），如果不带结束行，则显示开始行的上下5行。

### w(where)

w(where) - 打印当前执行堆栈，程序执行的位置。

### j(jump)

j(jump) 行 - 跳转到指定行。

### n(next)

n(next) - 单步执行。函数调用

### s(step)

s(step) - 进入函数内部。a(args) - 进入函数内部后，打印函数的所有参数。

### u(up)

u(up) - 跳回上一层调用，只是在代码层面跳转，程序执行到哪一步还是在哪一步，不信可以用n命令试一下。

### d(down) 

d(down) - 跳到调用的下一层，只是在代码层面跳转，程序执行到哪一步还是在哪一步，不信可以用n命令试一下。

### b(break) 

b(break)  [ ([filename:]lineno | function) [, condition] ] - 在指定文件（不指定，默认当前文件）指定行 打断点

### p(print)

p(print) 变量 - 打印变量的值pp(pretty print) 变量 - 格式化打印变量的值

### c(continue)

c(continue) - 继续运行，直到遇到下一个断点

### cl(clear) 

cl(clear) filename:lineno - 清除断点（只能清除交互调试中打的断点，在代码中set_trace()是无法清除的）

### r(return)

r(return) - 继续运行直到函数返回（函数内return语句，如果没有返回值，则跳到函数的最后）

### restart

restart - 重新启动调试器，断点等信息都会保留

### run

重新启动 debug，相当于 restart unt lineno

### until, lineno

until, lineno 是代码行号，持续执行直到运行到指定行（或遇到断点）

### interact	

 interact 启动一个 python 的交互式解释器, 使用 ctrl+d 返回 ipdb

### q(quit)

q(quit) - 退出调试模式变量名 - 查看变量内容!变量名 - 如果代码中变量名与ipdb命令冲突，则使用 **!**  解决冲突。







