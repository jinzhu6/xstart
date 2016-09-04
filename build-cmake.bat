REM echo.
REM echo *****************************************************************************
REM echo CMAKE WILL NOW DO ITS JOB ...
REM echo *****************************************************************************
mkdir build
cd build
cmake .. -DMSVC_RUNTIME=static
cd ..
pause