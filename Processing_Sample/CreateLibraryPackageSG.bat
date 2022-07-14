rem This is an attempt to write a better version of CreateLibraryPackage. 
rem It will do the following:
rem - Compile all the libraries required (x86/x64 Debug/Release)
rem - Package them into the correct name
rem - Zip them up with the correct name
rem
rem It will not:
rem - Touch or delete any of your project files. At all. 
rem 

rem --- Compilation Phase ---  
rem - We assume that users have already installed the required VS2019, to the point where they can build from there
rem TODO


rem --- CProcessingPackage Phase ---
rmdir .\CProcessingPackage /s /q
mkdir .\CProcessingPackage
mkdir .\CProcessingPackage\CProcessing
mkdir .\CProcessingPackage\CProcessing\inc
mkdir .\CProcessingPackage\CProcessing\lib
mkdir .\CProcessingPackage\CProcessing\lib\x64
mkdir .\CProcessingPackage\CProcessing\lib\x86

rem x64
xcopy .\CProcessing\lib\x64\CProcessing.lib .\CProcessingPackage\CProcessing\lib\x64 /s /r /y /q
xcopy .\CProcessing\lib\x64\CProcessing.dll .\CProcessingPackage\CProcessing\lib\x64 /s /r /y /q
xcopy .\CProcessing\lib\x64\CProcessingd.lib .\CProcessingPackage\CProcessing\lib\x64 /s /r /y /q
xcopy .\CProcessing\lib\x64\CProcessingd.dll .\CProcessingPackage\CProcessing\lib\x64 /s /r /y /q
xcopy .\CProcessing\lib\x64\fmod.dll .\CProcessingPackage\CProcessing\lib\x64 /s /r /y /q
xcopy .\CProcessing\lib\x64\fmodL.dll .\CProcessingPackage\CProcessing\lib\x64 /s /r /y /q

rem x86
xcopy .\CProcessing\lib\Win32\CProcessing.lib .\CProcessingPackage\CProcessing\lib\x86 /s /r /y /q
xcopy .\CProcessing\lib\Win32\CProcessing.dll .\CProcessingPackage\CProcessing\lib\x86 /s /r /y /q
xcopy .\CProcessing\lib\Win32\CProcessingd.lib .\CProcessingPackage\CProcessing\lib\x86 /s /r /y /q
xcopy .\CProcessing\lib\Win32\CProcessingd.dll .\CProcessingPackage\CProcessing\lib\x86 /s /r /y /q
xcopy .\CProcessing\lib\Win32\fmod.dll .\CProcessingPackage\CProcessing\lib\x86 /s /r /y /q
xcopy .\CProcessing\lib\Win32\fmodL.dll .\CProcessingPackage\CProcessing\lib\x86 /s /r /y /q

rem inc
xcopy .\CProcessing\inc\cprocessing.h .\CProcessingPackage\CProcessing\inc /s /r /y /q
xcopy .\CProcessing\inc\cprocessing_common.h .\CProcessingPackage\CProcessing\inc /s /r /y /q

rem --- Zipping phase --- 
rem I hope you are on Powershell v5 or above...
pushd .\CProcessingPackage
powershell Compress-Archive -LiteralPath "CProcessing" -DestinationPath "CProcessing.zip"
popd
