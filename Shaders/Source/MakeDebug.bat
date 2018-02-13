@echo off
for %%f in (*.vert *.frag) do "%VK_SDK_PATH%\bin\glslc.exe" %%f -o ../%%f.spv
pause