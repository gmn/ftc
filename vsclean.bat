@echo off

set EXE=ftc.exe 

if exist *.ilk del *.ilk 
if exist *.ncb del *.ncb 
if exist *.suo del /ah *.suo
if exist *.user del *.user
if exist *.pdb del *.pdb
if exist Debug rmdir /S /Q Debug
if exist Release rmdir /S /Q Release
if exist %EXE% del %EXE%
if exist *.manifest del *.manifest
if exist *.aps del *.aps
echo.
echo ************************************
echo.
echo     removing visual studio cruft
echo.
echo ************************************
echo.

:out

