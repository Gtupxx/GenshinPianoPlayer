# 原神自动弹琴机

## 仅供学习参考 ！不保证一定不被封号（至少我没被封）！

## 项目说明

- `song/*/` 文件夹下存放乐谱，在cpp文件里改路径名
- `auto.cpp`为未优化版本，使用了两个线程分别演奏和弦和主音
- `singleThread.cpp`是最终版，编译运行后输入s进入单线程模式，m进入多线程模式
- 编译的时候需要添加 "-lwinmm"，我使用下面的命令行编译：
- g++ D:\Code\ccpp\MIDI\singleThread.cpp -o D:\Code\ccpp\MIDI\bin\singleThread.exe -g -O3 -Wall -static-libgcc -std=c++20 -lwinmm 

## 乐谱编写

- `DORIAN`宏对应了原神的镜花之琴，前两个八度是多利亚调式，第三个八度是弗里吉亚调式

- 还没完全适配好，开启这个宏容易导致音符缺失

- `CDEFGAB`对应`Do，Re，Mi，Fa，Sol，La，Si`，使用`1，2，3`跟在音名后表示第1，2，3个八度，例如`G2`表示第二个八度的`Sol`

- 一行由音符名+时值组成，时值使用**一个整数**表示几分音符，例如`B2 4`表示第二个八度的`Si`音持续一个**四分音符**的时值

- `LL`表示**小节线**，后面的数字表示接下来是第几个小节

- `H0`表示空音，等同于一个**占位符**的作用

- 当需要同时演奏多个音符时，将其中一个音符写在最后，时值为整体的时值，其余的音符写在这个音符之前，时值写作0，例如：

  ``` none
  ...
  LL	13
  A1 0
  C2 0
  E2 4
  H0 16
  ...
  ```

  - 节选自FlowerDance的主音部分第13小节
  - 其中`A1`，`C2`，`E2`是**同时奏响的四分音符**,`H0`用作占位一个十六分音符

演示视频：【如何使用C++在原神里演奏《Flower Dance》？】https://www.bilibili.com/video/BV12a4y1U7GG?vd_source=9c1684d4e24f3e479137b280cb300191
