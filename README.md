# file transfer server

## C实现的文件传输服务端

支持断点续传(主要在客户端实现记录)

[C#客户端](ftc)

[学校协会的下载站](https://github.com/CodFrm/jx)

## Build

```

apt-get install libmysqlclient-dev  #Ubuntu

//yum install mysql-devel-i386      #Centos

Centos中Makefile中要修改

param=-lpthread -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient

make

./main

```

