@echo off 2>nul 3>nul
title ��ͼ�༭��
mode con cols=50 lines=30
::image�����ڴ�
%1start "" "tools\cmd.exe" "/c %~fs0 :"&exit
color 79
setlocal enabledelayedexpansion
set /p a_bmp=������bmp��
set /p map=������Ҫ�༭�ĵ�ͼ:
echo ������ͼ��...
if not exist %map%.dat echo.>>%map%.dat
set b_bmp=data\image\%a_bmp%.bmp
echo ���ڵ�����Դ�ļ�
set image=load %b_bmp% %a_bmp%
set x=36
set y=33
set ����id=0
::ִ���ⲿָ��
set save_load=f
for /f "tokens=* delims= " %%a in (data/boot.dat) do (
	%%a
	echo %%a
)
::��ʼ����ͼ
for /l %%a in (0,1,255) do (
	set ����_%%a=0
)
echo.
echo.
echo "=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-"
echo.                 ��ͼ�༭�� BY LHW
echo.     wsad �ƶ�����  q ���¼���Ŀ�귽��
echo.     o ������ͼ�ļ� e �ѷ���д���ͼ����
echo "=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-=+-" 
:run
set image=cls
set image=stretch bg 256 256
set image=bg 36 33 trans
::���ض���������ִ��imageָ��
set save_load=f
for /f "tokens=* delims= " %%a in (%map%.dat) do (
	%%a
)
set image=%a_bmp% %x% %y% 
set /a mx=x-36
set /a my=y-33
set /a bx=mx/16
set /a by=my/16
title MAP:%map% map_xy: (%mx%,%my%) block_xy: (%bx%,%by%) id:!����_%����id%!
choice /c wsadeqo /n >nul

if %errorlevel%==1 (
	set /a y-=16
	set /a ����id-=16
)
if %errorlevel%==2 (
	set /a y+=16
	set /a ����id+=16
)
if %errorlevel%==3 (
	set /a x-=16
	set /a ����id-=1
)
if %errorlevel%==4 (
	set /a x+=16
	set /a ����id+=1
)
if %errorlevel%==5 (
call :д�뷽��
)
if %errorlevel%==6 (
	call :�����ز�
)
if %errorlevel%==7 (
	call :�����ͼ
)
goto run
:�����ز�
set a_bmp=
set /p a_bmp=������bmp��
set b_bmp=data\image\%a_bmp%.bmp
set image=load %b_bmp% %a_bmp%
Goto :Eof
:д�뷽��
set ����_%����id%=%a_bmp%
echo set ����_%����id%=%a_bmp% >>%map%.dat
echo set image=%a_bmp% %x% %y% >>%map%.dat
goto :Eof
:�����ͼ
if exist "MAP_%map%.dat" del MAP_%map%.dat /s/f/q
for /l %%a in (0,1,255) do (
	echo.!����_%%a!,>>MAP_%map%.dat
)
set n=
for /f "tokens=*" %%i in (MAP_%map%.dat) do set n=!n! %%i
del MAP_%map%.dat
echo ^/^/Room %map% >>MAP_%map%.dat
echo %n%>>MAP_%map%.dat
start "" MAP_%map%.dat
goto :Eof