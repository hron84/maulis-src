
To build test program:
$ gcc -o udir udir.c

To build shareable lib:
$ gcc --share -ldl -fPIC -o getpwnam_fake.so getpwnam_fake.c

To run test program:

$ ./udir root
homedir of user root is /root
$ ./udir nobody
homedir of user nobody is /nonexistent

To run test program with lib:

$ echo root nobody >.getpwnam_fake
$ LD_PRELOAD=./getpwnam_fake.so ./udir root
homedir of user root is /nonexistent

