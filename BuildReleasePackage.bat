@ECHO off
REM CProcessing - automate build and prepare release files

SETLOCAL
GOTO :init

:header
    ECHO %__NAME%
    ECHO CProcessing - script to automate build and prepare release files.
    ECHO.
    GOTO :eof

:usage
    ECHO USAGE:
    ECHO   %__BAT_NAME% [flags]
	ECHO.  Example:
	ECHO.  %__BAT_NAME% -r all -c -z
    ECHO.
    ECHO.  /?, --help           show this help
    ECHO.  /d, --debug          show debug output
	ECHO.  /b, --build target   build "all" or specifiy a target and platform, "Debug|x64"
	ECHO.  /r, --rebuild target rebuild "all" or specifiy a target and platform, "Debug|x64"
	ECHO.  /c, --copy           copy the inc and lib files to the Empty project
	ECHO.  /z, --zip            zip the Empty project into a release package
    GOTO :eof

:argument_issue
    CALL :header
    CALL :usage
    ECHO.
	IF DEFINED CP_Script_Unrecognized (
		ECHO **  Unrecognized argument: "%CP_Script_Unrecognized%"  **
		ECHO.
		GOTO :eof
	)
    IF NOT DEFINED CP_Script_EmptyArgs (
		ECHO **  No arguments specified  **
	)
    ECHO.
    GOTO :eof

:init
    SET "__NAME=%~n0"
    set "__BAT_NAME=%~nx0"

    SET "CP_Script_Debug="
	SET "CP_Script_Build="
	SET "CP_Script_Rebuild="
    SET "CP_Script_Copy="
	SET "CP_Script_Zip="
    SET "CP_Script_Unrecognized="
	SET "CP_Script_EmptyArgs="

:parse
    IF "%~1"=="" GOTO :validate
	
	IF NOT DEFINED CP_Script_EmptyArgs        SET "CP_Script_EmptyArgs=no"

    IF /i "%~1"=="/?"         CALL :header & CALL :usage "%~2" & GOTO :end
    IF /i "%~1"=="-?"         CALL :header & CALL :usage "%~2" & GOTO :end
    IF /i "%~1"=="-h"         CALL :header & CALL :usage "%~2" & GOTO :end
    IF /i "%~1"=="--help"     CALL :header & CALL :usage "%~2" & GOTO :end

    IF /i "%~1"=="/d"         SET "CP_Script_Debug=yes"    & SHIFT & GOTO :parse
    IF /i "%~1"=="-d"         SET "CP_Script_Debug=yes"    & SHIFT & GOTO :parse
    IF /i "%~1"=="--debug"    SET "CP_Script_Debug=yes"    & SHIFT & GOTO :parse

    IF /i "%~1"=="/b"         SET "CP_Script_Build=%~2"    & SHIFT & SHIFT & GOTO :parse
    IF /i "%~1"=="-b"         SET "CP_Script_Build=%~2"    & SHIFT & SHIFT & GOTO :parse
    IF /i "%~1"=="--build"    SET "CP_Script_Build=%~2"    & SHIFT & SHIFT & GOTO :parse

    IF /i "%~1"=="/r"         SET "CP_Script_Rebuild=%~2"  & SHIFT & SHIFT & GOTO :parse
    IF /i "%~1"=="-r"         SET "CP_Script_Rebuild=%~2"  & SHIFT & SHIFT & GOTO :parse
    IF /i "%~1"=="--rebuild"  SET "CP_Script_Rebuild=%~2"  & SHIFT & SHIFT & GOTO :parse

    IF /i "%~1"=="/c"         SET "CP_Script_Copy=yes"     & SHIFT & GOTO :parse
    IF /i "%~1"=="-c"         SET "CP_Script_Copy=yes"     & SHIFT & GOTO :parse
    IF /i "%~1"=="--copy"     SET "CP_Script_Copy=yes"     & SHIFT & GOTO :parse

    IF /i "%~1"=="/z"         SET "CP_Script_Zip=yes"      & SHIFT & GOTO :parse
    IF /i "%~1"=="-z"         SET "CP_Script_Zip=yes"      & SHIFT & GOTO :parse
    IF /i "%~1"=="--zip"      SET "CP_Script_Zip=yes"      & SHIFT & GOTO :parse

    IF NOT DEFINED CP_Script_Unrecognized     SET "CP_Script_Unrecognized=%~1"    & SHIFT & GOTO :validate

    SHIFT
    GOTO :parse

:validate
	IF NOT DEFINED CP_Script_EmptyArgs GOTO :argument_issue & GOTO :end
	IF DEFINED CP_Script_Unrecognized GOTO :argument_issue & GOTO :end

:main
	IF DEFINED CP_Script_Debug (
		@ECHO ON
		ECHO **  Debug output is enabled  **
	)

	SET "CP_Script_BuildFlag="
	SET "CP_Script_BuildTarget="

	IF DEFINED CP_Script_Build (
		SET "CP_Script_BuildFlag=build"
		SET "CP_Script_BuildTarget=%CP_Script_Build%"
	)
	IF DEFINED CP_Script_Rebuild (
		IF DEFINED CP_Script_Build ECHO **  Both Build and Rebuild arguments found, defaulting to Rebuild  **
		SET "CP_Script_BuildFlag=rebuild"
		SET "CP_Script_BuildTarget=%CP_Script_Rebuild%"
	)

	IF "%CP_Script_BuildTarget%"=="all" (
		ECHO.
		ECHO **  Building all targets and platforms  **
		FOR %%t in ("Debug|x86" "Release|x86" "Debug|x64" "Release|x64") do (
			ECHO.
			ECHO Calling devenv with the following parameters: /%CP_Script_BuildFlag% %%t
			devenv .\Processing_Sample\Processing_Sample.sln /%CP_Script_BuildFlag% %%t
		)
	) ELSE IF DEFINED CP_Script_BuildTarget (
		ECHO.
		ECHO Building specific target platform, devenv parameters: /%CP_Script_BuildFlag% "%CP_Script_BuildTarget%"
		devenv .\Processing_Sample\Processing_Sample.sln /%CP_Script_BuildFlag% "%CP_Script_BuildTarget%"
	)

	IF DEFINED CP_Script_Copy (
		ECHO.
		ECHO **  Copying inc and lib to the Empty project  **
		REM collect inc and lib folders and remove extra files

		IF NOT EXIST .\CProcessingOutput MKDIR .\CProcessingOutput
		DEL .\CProcessingOutput\*.* /f /q
		XCOPY .\Processing_Sample\CProcessing\inc\* .\CProcessingOutput\inc\ /s /r /y /q
		XCOPY .\Processing_Sample\CProcessing\lib\* .\CProcessingOutput\lib\ /s /r /y /q
		CD .\CProcessingOutput\lib\x64
		FOR %%e in (exp iobj ipdb pdb ilk) do (
			IF EXIST *.%%e DEL *.%%e
		)
		CD ..\x86
		FOR %%e in (exp iobj ipdb pdb ilk) do (
			IF EXIST *.%%e DEL *.%%e
		)
		CD ..\..\inc
		DEL fmod*.*
		DEL glfw*.*
		CD ..\..\

		REM clear empty project and copy latest inc and lib to the Empty project
		IF NOT EXIST .\Processing_Empty\CProcessing\ MKDIR .\Processing_Empty\CProcessing\
		DEL .\Processing_Empty\CProcessing\*.* /f /q
		XCOPY .\CProcessingOutput\* .\Processing_Empty\CProcessing\ /s /r /y /q
	)

	IF DEFINED CP_Script_Zip (
		ECHO.
		ECHO ** Zip Empty project and save to Releases **

		IF EXIST .\Releases RMDIR .\Releases /s /q
		MKDIR .\Releases\CProcessingTemplate\
		FOR %%i in (Assets\ CProcessing\) do (
			XCOPY ".\Processing_Empty\%%i" ".\Releases\CProcessingTemplate\%%i" /s /y /q
		)
		FOR %%i in (main.c Processing_Sample.sln Processing_Sample.vcxproj) do (
			XCOPY ".\Processing_Empty\%%i" ".\Releases\CProcessingTemplate\" /s /y /q
		)
		powershell Compress-Archive .\Releases\CProcessingTemplate\* .\Releases\CProcessingTemplate.zip

		REM cleanup uncompressed folder
		RMDIR .\Releases\CProcessingTemplate\ /s /q
	)

:end
    EXIT /B
