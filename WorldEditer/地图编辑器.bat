@echo off 2>nul 3>nul
title 地图编辑器
mode con cols=50 lines=30
::image载入内存
%1start "" "tools\cmd.exe" "/c %~fs0 :"&exit
color 79
setlocal enabledelayedexpansion
set /p a_bmp=请输入bmp：
set /p map=请输入要编辑的地图:
echo 创建地图中...
if not exist %map%.dat echo.>>%map%.dat
set b_bmp=data\image\%a_bmp%.bmp
echo 正在导入资源文件
set image=load %b_bmp% %a_bmp%
set x=36
set y=33
set 方块id=0
::执行外部指令
set save_load=f
for /f "tokens=* delims= " %%a in (data/boot.dat) do (
	%%a
	echo %%a
)
::初始化地图
for /l %%a in (0,1,255) do (
	set 方块_%%a=0
)
echo.
echo.
echo "=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-"
echo.                 地图编辑器 BY LHW
echo.     wsad 移动方块  q 重新加载目标方块
echo.     o 导出地图文件 e 把方块写入地图工程
echo "=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-" 
:run
set image=cls
set image=stretch bg 256 256
set image=bg 36 33 trans
::加载动画和批量执行image指令
set save_load=f
for /f "tokens=* delims= " %%a in (%map%.dat) do (
	%%a
)
set image=%a_bmp% %x% %y% 
set /a mx=x-36
set /a my=y-33
set /a bx=mx/16
set /a by=my/16
title MAP:%map% map_xy: (%mx%,%my%) block_xy: (%bx%,%by%) id:!方块_%方块id%!
choice /c wsadeqo /n >nul

if %errorlevel%==1 (
	set /a y-=16
	set /a 方块id-=16
)
if %errorlevel%==2 (
	set /a y+=16
	set /a 方块id+=16
)
if %errorlevel%==3 (
	set /a x-=16
	set /a 方块id-=1
)
if %errorlevel%==4 (
	set /a x+=16
	set /a 方块id+=1
)
if %errorlevel%==5 (
call :写入方块
)
if %errorlevel%==6 (
	call :重载素材
)
if %errorlevel%==7 (
	call :输出地图
)
goto run
:重载素材
set a_bmp=
set /p a_bmp=请输入bmp：
set b_bmp=data\image\%a_bmp%.bmp
set image=load %b_bmp% %a_bmp%
Goto :Eof
:写入方块
set 方块_%方块id%=%a_bmp%
echo set 方块_%方块id%=%a_bmp% >>%map%.dat
echo set image=%a_bmp% %x% %y% >>%map%.dat
goto :Eof
:输出地图
if exist "MAP_%map%.dat" del MAP_%map%.dat /s/f/q
for /l %%a in (0,1,255) do (
	echo.!方块_%%a!,>>MAP_%map%.dat
)
set n=
for /f "tokens=*" %%i in (MAP_%map%.dat) do set n=!n! %%i
del MAP_%map%.dat
echo ^/^/Room %map% >>MAP_%map%.dat
echo %n%>>MAP_%map%.dat
start "" MAP_%map%.dat
goto :Eof