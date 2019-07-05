@echo off 2>nul 3>nul
title Arduboy音乐生成器 %档案名%
mode con cols=50 lines=30
set /p 档案名=请输入档案名:
title Arduboy音乐生成器 - %档案名%
:loop
set /p 音调=音调  
set /p 间隔=间隔  
:logic
if "%音调%"=="0" set 写入音调=0
if "%音调%"=="1" set 写入音调=262
if "%音调%"=="2" set 写入音调=294
if "%音调%"=="3" set 写入音调=330
if "%音调%"=="4" set 写入音调=349
if "%音调%"=="5" set 写入音调=392
if "%音调%"=="6" set 写入音调=440
if "%音调%"=="7" set 写入音调=497
if "%音调%"=="8" set 写入音调=523
if "%音调%"=="9" set 写入音调=587
if "%音调%"=="+"set 写入音调=659
if "%音调%"=="-" set 写入音调=698
if "%音调%"=="*" set 写入音调=784
if "%音调%"=="/" set 写入音调=880
if "%音调%"=="=" set 写入音调=988
:write
echo %写入音调%,%间隔%,>>%档案名%.ArduboyTones.txt
echo %音调%,>>%档案名%.build.txt
echo %写入音调%,>>%档案名%.Tones.txt
echo %间隔%,>>%档案名%.delay.txt
echo %写入音调%,%间隔%,
echo.
goto loop