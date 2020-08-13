## Git 基本操作

- git init  # 初始化仓库
- git add * (git add <filename>)  # 提交文件到暂存区
	- git add -A # 提交所有变化  
	- git add -u # 提交被修改和被删除的文件，不包括新文件  
	- git add . # 提交新文件和被修改的文件，不包括被删除文件  
- git commit -m "your reason"   # 向远程仓库提交文件但没有上传到远程仓库
- git status     # 查看暂存区状态
- git pull    # 将本地的仓库与远程仓库同步
- git remote add origin <repository link> # 将本地仓库与某一个远程仓库连接，让后在本地仓库 git push 就可以把文件上传到远程仓库
  - (git remote rm origin # delete origin)    
- git push -u origin master(git push origin master) # 已经执行了 git commit -m "your reason" 后，将本地仓库的改动文件都上传到远程仓库
- git reset HEAD~  # 撤销领先的最近一次提交 ，删除操作还有待研究


