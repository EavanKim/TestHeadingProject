REM @echo off

SET "START_PATH=%~dp0"

SET REVERT_PATH_LIST=%START_PATH%..\HeadingNet
SET REVERT_PATH_LIST=%REVERT_PATH_LIST%;%START_PATH%..\TestHeadingProject

FOR %%i IN (%REVERT_PATH_LIST%) DO (
	PUSHD %%i
		git reset --hard HEAD
		git pull
	POPD
)

PAUSE
