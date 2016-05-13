@echo off

REM call build-clean.bat

REM echo.
REM echo *****************************************************************************
REM echo CMAKE WILL NOW DO ITS JOB ...
REM echo *****************************************************************************
REM call build.bat
REM cd ..


echo.
echo.
echo *****************************************************************************
echo PLEASE WAIT WHILE THE SOFTWARE IS BUILD, THIS MAY TAKE A FEW MINUTES ...
echo *****************************************************************************
echo Building release version software ....
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.com" .\build\xstart.sln /BUILD Release
if %ERRORLEVEL% GEQ 1 goto ONERROR
timeout 3


echo.
echo.
echo *****************************************************************************
echo UPDATING DOCUMENTATION AND AUTO-COMPLETION ...
echo *****************************************************************************
copy .\bin\Release\xstart.exe .\bin\xstart.exe
copy .\libad4.dll .\bin\libad4.dll
copy .\COPYING .\bin\COPYING
copy .\editor\format.bat .\bin\
copy .\editor\astyle.exe .\bin\
cd scripts
..\bin\xstart docgen.gm
..\bin\xstart apigen.gm
cd ..



echo.
echo.
echo *****************************************************************************
echo BUILDING INSTALLER NOW, FIND THE "xstart-setup" IN THE "_SETUP" PATH ...
echo *****************************************************************************
echo Building installer ....
"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" "SETUP.ISS" /F"xstart-setup-latest" /Q
if %ERRORLEVEL% GEQ 1 goto ONERROR
mkdir H:\Transfer\pub\software
mkdir H:\Transfer\pub\software\xstart
copy _SETUP\xstart-setup-latest.exe H:\Transfer\pub\software\xstart
copy _SETUP\xstart-setup-latest.exe H:\Software\Software Tools\Udoo


echo.
echo.
echo *****************************************************************************
echo YOU MAY NOW INSTALL A FRESH COPY ON YOUR LOCAL PC ....
echo *****************************************************************************
_SETUP\xstart-setup-latest.exe /VERYSILENT

pause
exit


:ONERROR
echo.
echo.
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo   BUILD FAILED! 
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
pause
exit

