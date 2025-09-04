@echo off
echo ========================================
echo    FIXING REPOSITORY COMPLETELY
echo ========================================
echo.

echo Step 1: Untracking DLL files from LFS...
git lfs untrack "*.dll"
if %errorlevel% neq 0 (
    echo ERROR: Failed to untrack DLL files from LFS
    pause
    exit /b 1
)

echo.
echo Step 2: Removing DLL files from LFS tracking...
git rm --cached "Plugins/BeamEyeTracker/ThirdParty/BeamSDK/bin/win64/*.dll"
if %errorlevel% neq 0 (
    echo ERROR: Failed to remove DLL files from LFS
    pause
    exit /b 1
)

echo.
echo Step 3: Adding DLL files back to regular Git tracking...
git add "Plugins/BeamEyeTracker/ThirdParty/BeamSDK/bin/win64/*.dll"
if %errorlevel% neq 0 (
    echo ERROR: Failed to add DLL files to Git
    pause
    exit /b 1
)

echo.
echo Step 4: Amending commit to use consistent message...
git commit --amend -m "v1 testing repo setup"
if %errorlevel% neq 0 (
    echo ERROR: Failed to amend commit
    pause
    exit /b 1
)

echo.
echo Step 5: Checking final status...
git status

echo.
echo ========================================
echo    REPOSITORY FIXED SUCCESSFULLY!
echo ========================================
echo.
echo You can now push without LFS budget issues:
echo   git push origin main
echo.
pause
