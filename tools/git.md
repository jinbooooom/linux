## Git 基本操作

### 一些概念

**工作区：**电脑上你能看到的目录

**版本库：**工作区的隐藏目录`.git`。含

- **暂存区：**`git add`后但未`git commit`的文件修改被添加到暂存区
- **本地分支：**`git commit`后，但未`git push`，即把暂存区的所有内容提交到本地分支

[练习 git 的在线网站](https://learngitbranching.js.org/?NODEMO=&locale=zh_CN)

## Git 基本操作

### 初始化仓库

```bash
# 下载一个 github 仓库
$ git clone [url]

# 初始化仓库
$ git init

# 新建一个目录，将其初始化为 Git 代码库
$ git init [project-name]
```

### 配置

```bash
# 显示当前的 Git 配置
$ git config --list

# 编辑 Git 配置文件
$ git config -e 
```

### 增加/删除文件

```bash
# 添加指定文件到暂存区
$ git add [file1] [file2] ...

# 添加指定目录到暂存区，包括子目录
$ git add [dir]

# 添加当前目录的所有文件到暂存区
$ git add -A 

# 提交新文件和被修改的文件，不包括被删除的文件  
$ git add .

# 提交被修改和被删除的文件，不包括新文件  
$ git add -u

# 停止跟踪指定文件，该文件会保留在工作区
# 停止跟踪后修改了停止跟踪的文件，并且 git add 到暂存区，又会再次跟踪
$ git rm --cached [file1] [file2]...

# 停止跟踪指定文件夹（可含子文件夹），该文件会保留在工作区
$ git rm --cached -r [dir]

# 删除工作区文件，使用 git 的这个命令不如用 shell 的 rm 命令
$ git rm -f [file1] [file2] ...

# 重命名暂存区文件（同时重命名工作区的文件名），重命名的文件被放入暂存区
$ git mv [file-original] [file-renamed]
```

### 代码提交

```bash
# 提交暂存区文件到本地仓库
$ git commit -m [message]

# 提交暂存区的指定文件到仓库区
$ git commit [file1] [file2] ... -m [message]

# 使用一次新的 commit，替代上一次提交
# 如果代码没有任何新变化，则用来改写上一次 commit 的提交信息
$ git commit --amend -m [message]
```

### 分支

```bash
# 列出所有本地分支
$ git branch

# 列出所有远程分支
$ git branch -r

# 列出所有本地分支和远程分支
$ git branch -a

# 新建一个分支，但依然停留在当前分支
$ git branch [branch-name]

# 新建一个分支，并切换到该分支
$ git checkout -b [branch-name]

# 新建一个分支，与指定的远程分支建立追踪关系
$ git branch --track [branch-name] [remote-branch-name]

# 切换到指定分支，并更新工作区
$ git checkout [branch-name]

# 合并指定分支到当前分支
$ git merge [branch-name]

# 删除分支
$ git branch -d [branch-name]

# 删除远程分支
$ git push origin --delete <branch-name>
$ git branch -dr <remote/branch>
```

### 查看信息

```bash
# 查看暂存区状态
$ git status

# 显示当前分支的版本历史
$ git log

# 显示commit历史，以及每次commit发生变更的文件
$ git log --stat

# 显示暂存区和工作区的差异
$ git diff

# 显示工作区与当前分支最新commit之间的差异
$ git diff HEAD

# 显示某次提交的数据和内容变化
$ git show [commit]

# 显示某次提交时，某个文件的内容
$ git show [commit]:[filename]

# 显示当前分支的最近几次提交
$ git reflog
```

### 远程同步

```bash
# 下载远程仓库的所有变动
$ git fetch [remote]

# 显示所有远程仓库
$ git remote -v

# 取回远程仓库的变化，并与本地分支合并
$ git pull [remote] [branch]

# 上传本地指定分支到远程仓库
$ git push [remote] [branch]

# 强行推送当前分支到远程仓库，即使有冲突
$ git push [remote] --force

# 推送所有分支到远程仓库
$ git push [remote] --all
```

### 撤销

```bash
# 恢复暂存区的指定文件到工作区
$ git checkout [file]

# 恢复某个 commit 的指定文件到工作区
$ git checkout [commit] [file]

# 恢复上一个 commit 的所有文件到工作区
$ git checkout .

# 重置暂存区的指定文件，与上一次 commit 保持一致，但工作区不变
$ git reset [file]

# 重置暂存区与工作区，与上一次 commit 保持一致
$ git reset --hard

# 重置当前分支的指针为指定 commit，同时重置暂存区，但工作区不变
$ git reset [commit]

# 重置当前分支的 HEAD 为指定 commit，同时重置暂存区和工作区，与指定commit 一致
$ git reset --hard [commit]

# 重置当前 HEAD 为指定 commit，但保持暂存区和工作区不变
$ git reset --keep [commit]

# 新建一个 commit，用来撤销指定 commit
# 后者的所有变化都将被前者抵消，并且应用到当前分支
$ git revert [commit]
```

## 其他
