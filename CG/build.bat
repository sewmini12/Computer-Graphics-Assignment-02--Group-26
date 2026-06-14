@echo off
echo =========================================
echo   Building 3D City Explorer...
echo =========================================

"C:\msys64\ucrt64\bin\g++.exe" -o CityExplorer.exe ^
    main.cpp globals.cpp player.cpp camera.cpp ^
    environment.cpp collision.cpp lighting.cpp ^
    mission.cpp ui.cpp ^
    -I"C:/msys64/ucrt64/include" ^
    -L"C:/msys64/ucrt64/lib" ^
    -lfreeglut -lopengl32 -lglu32 ^
    -mwindows ^
    2>&1

IF %ERRORLEVEL% EQU 0 (
    copy /Y "C:\msys64\ucrt64\bin\libfreeglut.dll" . >nul
    echo.
    echo [SUCCESS] Build complete! Running game...
    echo.
    start CityExplorer.exe
) ELSE (
    echo.
    echo [FAILED]  Build errors above. Fix and retry.
    echo.
    echo If freeglut is missing, install it in MSYS2:
    echo   pacman -S mingw-w64-ucrt-x86_64-freeglut
    pause
)
