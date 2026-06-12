@echo off
echo =========================================
echo   Building 3D City Explorer...
echo =========================================

g++ -o CityExplorer.exe ^
    main.cpp globals.cpp player.cpp camera.cpp ^
    environment.cpp collision.cpp lighting.cpp ^
    mission.cpp ui.cpp ^
    -I"C:/mingw64/include" ^
    -L"C:/mingw64/lib" ^
    -lfreeglut -lopengl32 -lglu32 ^
    -mwindows ^
    2>&1

IF %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] Build complete! Running game...
    echo.
    start CityExplorer.exe
) ELSE (
    echo.
    echo [FAILED]  Build errors above. Fix and retry.
    pause
)
