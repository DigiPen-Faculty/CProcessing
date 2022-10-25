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


rem --- CProcessingPackageSG Phase ---
rmdir .\CProcessingPackageSG /s /q
mkdir .\CProcessingPackageSG
mkdir .\CProcessingPackageSG\CProcessing
mkdir .\CProcessingPackageSG\CProcessing\inc
mkdir .\CProcessingPackageSG\CProcessing\lib
mkdir .\CProcessingPackageSG\CProcessing\lib\x64
mkdir .\CProcessingPackageSG\CProcessing\lib\x86

rem x64
xcopy .\CProcessing\lib\x64\CProcessing.lib .\CProcessingPackageSG\CProcessing\lib\x64 /s /r /y /q
xcopy .\CProcessing\lib\x64\CProcessing.dll .\CProcessingPackageSG\CProcessing\lib\x64 /s /r /y /q
xcopy .\CProcessing\lib\x64\CProcessingd.lib .\CProcessingPackageSG\CProcessing\lib\x64 /s /r /y /q
xcopy .\CProcessing\lib\x64\CProcessingd.dll .\CProcessingPackageSG\CProcessing\lib\x64 /s /r /y /q
xcopy .\CProcessing\lib\x64\fmod.dll .\CProcessingPackageSG\CProcessing\lib\x64 /s /r /y /q
xcopy .\CProcessing\lib\x64\fmodL.dll .\CProcessingPackageSG\CProcessing\lib\x64 /s /r /y /q

rem x86
xcopy .\CProcessing\lib\x86\CProcessing.lib .\CProcessingPackageSG\CProcessing\lib\x86 /s /r /y /q
xcopy .\CProcessing\lib\x86\CProcessing.dll .\CProcessingPackageSG\CProcessing\lib\x86 /s /r /y /q
xcopy .\CProcessing\lib\x86\CProcessingd.lib .\CProcessingPackageSG\CProcessing\lib\x86 /s /r /y /q
xcopy .\CProcessing\lib\x86\CProcessingd.dll .\CProcessingPackageSG\CProcessing\lib\x86 /s /r /y /q
xcopy .\CProcessing\lib\x86\fmod.dll .\CProcessingPackageSG\CProcessing\lib\x86 /s /r /y /q
xcopy .\CProcessing\lib\x86\fmodL.dll .\CProcessingPackageSG\CProcessing\lib\x86 /s /r /y /q

rem inc
xcopy .\CProcessing\inc\cprocessing.h .\CProcessingPackageSG\CProcessing\inc /s /r /y /q
xcopy .\CProcessing\inc\cprocessing_common.h .\CProcessingPackageSG\CProcessing\inc /s /r /y /q

rem --- Zipping phase --- 
rem I hope you are on Powershell v5 or above...
pushd .\CProcessingPackageSG
powershell Compress-Archive -LiteralPath "CProcessing" -DestinationPath "CProcessingSG.zip"
popd
