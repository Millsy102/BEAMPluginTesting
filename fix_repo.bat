@echo off
echo Fixing repository issues...

echo.
echo 1. Untracking DLL files from LFS...
git lfs untrack "*.dll"

echo.
echo 2. Removing DLL files from LFS tracking...
git rm --cached "Plugins/BeamEyeTracker/ThirdParty/BeamSDK/bin/win64/*.dll"

echo.
echo 3. Adding DLL files back to regular Git tracking...
git add "Plugins/BeamEyeTracker/ThirdParty/BeamSDK/bin/win64/*.dll"

echo.
echo 4. Checking current status...
git status

echo.
echo 5. Amending commit to use consistent message...
git commit --amend -m "v1 testing repo setup"

echo.
echo 6. Checking final status...
git status

echo.
echo Repository fixed! You should now be able to push without LFS budget issues.
pause
