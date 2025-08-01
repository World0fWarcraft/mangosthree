@echo off
:main
if not exist ..\src\tools\Extractor_projects\.git goto extractors:
if not exist ..\dep\.git goto dep:
if not exist ..\src\realmd\.git goto realm:
goto endpoint:

:extractors
echo Patching Extractors
copy Patch_Easybuild_Mangos3.cmd ..\src\tools\Extractor_projects\.git
goto main:

:dep
echo Patching Dep
copy Patch_Easybuild_Mangos3.cmd ..\dep\.git
goto main:

:realm
echo Patching Realm
copy Patch_Easybuild_Mangos3.cmd ..\src\realmd\.git
goto main:

:endpoint