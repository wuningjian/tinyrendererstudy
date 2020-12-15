# tinyrendererstudy
lesson 0文件夹是lesson 1-5的集合，画线有噪点，失真，zbuffer也有点问题

## 由于msbuild依赖vs环境，需要在VS 2017的开发人员命令行中运行
## compilation
```sh
cd lesson0/source
mkdir build 
cd build 
cmake ..
msbuild Lesson0.sln 
./main/Debug/main.exe ../../model/african_obj.obj ../../model/african_head_diffuse.tga
```sh

lesson 5文件夹，开始重写之前的向量和矩阵库
## 这里开始使用Unix的编译,脱离VS的环境依赖
```sh
cd lesson5/source
mkdir build 
cd build 
cmake -G "Unix Makefiles" ..
make
./main/main.exe ../../model/african_obj.obj ../../model/african_head_diffuse.tga
```sh