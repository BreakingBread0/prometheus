@echo off
echo Bootstrapping vcpkg...
cd external\vcpkg
call .\bootstrap-vcpkg.bat
cd ..\..
echo vcpkg bootstrapped.