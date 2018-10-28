@echo off
set unrealPath=F:\Unreal\UE_4.20\Engine
set projPath=D:\sandbox\MFA_USC\Thesis
call "%unrealPath%\Build\BatchFiles\Build.bat" "EpidemicVectorsEditor" Win64 Development -WarningAsErrors "%projPath%\EpidemicVectors\EpidemicVectors.uproject"
PAUSE