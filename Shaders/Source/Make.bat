@echo off
for %%f in (*.vert *.frag) do "%VK_SDK_PATH%\bin\glslangValidator.exe" -V %%f -o ../%%f.spv
pause