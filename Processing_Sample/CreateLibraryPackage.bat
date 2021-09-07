cd .\CProcessing\lib\x64
del *.exp
del *.iobj
del *.ipdb
del *.pdb
del *.ilk
cd ..\x86
del *.exp
del *.iobj
del *.ipdb
del *.pdb
del *.ilk
cd ..\..\inc
del fmod*.*
del glfw*.*
cd ..\..\
mkdir .\CProcessingPackage
xcopy .\CProcessing\inc\* .\CProcessingPackage\inc\ /s /r /y /q
xcopy .\CProcessing\lib\* .\CProcessingPackage\lib\ /s /r /y /q