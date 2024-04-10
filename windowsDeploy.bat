
rem -----------------------------------------------------------------------------------
rem Depend on QT compiler
rem -----------------------------------------------------------------------------------

set MINGW_PATH="C:\Qt\Tools\mingw730_64\bin\"

set QT_PATH="C:\Qt\5.12.12\mingw73_64\bin\"
set DEPLOYCMD="%QT_PATH:~1,-1%windeployqt.exe"

set EXEPATH="..\build-calv-Desktop_Qt_5_12_12_MinGW_64_bit-Release\release\"

set OUTPATH="..\calv_executable"

rem -----------------------------------------------------------------------------------
rem DO NOT CHANGE BELOW
rem -----------------------------------------------------------------------------------

set EXE="%EXEPATH:~1,-1%calv.exe"

echo %EXE%


rd /s /q %OUTPATH%

mkdir %OUTPATH%

copy %EXE% %OUTPATH%
copy "%QT_PATH:~1,-1%libgcc_s_seh-1.dll" %OUTPATH%
copy "%QT_PATH:~1,-1%libstdc++-6.dll" %OUTPATH%
copy "%QT_PATH:~1,-1%libwinpthread-1.dll" %OUTPATH%

rem copy %EXEPATH%*.dll %OUTPATH%

%DEPLOYCMD% --dir %OUTPATH% --qmldir .\qml %EXE%
pause

