@assoc .lv1=
@ftype executable_process1=

@set CURRENT_PATH=%~dp0

@ftype executable_process1="%CURRENT_PATH%compile_lv1.bat" "%%1"
@assoc .lv1=executable_process1

@reg add "HKCR\executable_process1\shell\¢ x86\command" /f /t REG_SZ /d "\"%CURRENT_PATH%to_x86.bat\" \"%%1\""