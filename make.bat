@echo off
set fileName=WinMain
set appName=app
gcc -mwindows %fileName%.c -o %appName%.exe

