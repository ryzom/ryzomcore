rem This script copies lost interface graphics to the interface installation directory
xcopy /Y .\pub\assets\interfaces\ .\pipeline\install\interfaces\
copy .\pipeline\install\interfaces\login_bg.dds .\pipeline\install\interfaces\launcher_bg_1.dds
move .\pipeline\install\interfaces\login_bg.dds .\pipeline\install\interfaces\launcher_bg_0.dds
if /I "%RC_ROOT%"=="" pause
