# ROCE

## ROCEv2

两台服务器直连，一台Link layer是InfiniBand，另一台是Ethernet，那么用ibstat查看两台的状态就会都是PORT_DOWN (1)，如果当两台都切换为Ethernet，就会同时变为Active

![image-20230801100450053](assets/roce/image-20230801100450053.png)

![image-20230801100513439](assets/roce/image-20230801100513439.png)

