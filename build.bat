@echo off
REM Builds the app and the test runner without make (needs g++ on PATH).
setlocal enabledelayedexpansion
set FILES=
for %%f in (src\common\*.cpp src\member2_ayaan\*.cpp src\member3_shamaem\*.cpp) do set FILES=!FILES! %%f
for %%f in (src\member1_saad\*.cpp) do if /I not "%%~nxf"=="main.cpp" set FILES=!FILES! %%f

g++ -std=c++17 -Wall -Wextra -O2 -Isrc -o campusrides.exe src\member1_saad\main.cpp %FILES% || exit /b 1
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -o test_runner.exe tests\*.cpp %FILES% || exit /b 1
echo Built campusrides.exe and test_runner.exe
