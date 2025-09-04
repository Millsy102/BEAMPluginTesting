Write-Host "Fixing repository issues..." -ForegroundColor Green

Write-Host "`n1. Untracking DLL files from LFS..." -ForegroundColor Yellow
git lfs untrack "*.dll"

Write-Host "`n2. Removing DLL files from LFS tracking..." -ForegroundColor Yellow
git rm --cached "Plugins/BeamEyeTracker/ThirdParty/BeamSDK/bin/win64/*.dll"

Write-Host "`n3. Adding DLL files back to regular Git tracking..." -ForegroundColor Yellow
git add "Plugins/BeamEyeTracker/ThirdParty/BeamSDK/bin/win64/*.dll"

Write-Host "`n4. Checking current status..." -ForegroundColor Yellow
git status

Write-Host "`n5. Amending commit to use consistent message..." -ForegroundColor Yellow
git commit --amend -m "v1 testing repo setup"

Write-Host "`n6. Checking final status..." -ForegroundColor Yellow
git status

Write-Host "`nRepository fixed! You should now be able to push without LFS budget issues." -ForegroundColor Green
Read-Host "Press Enter to continue"
