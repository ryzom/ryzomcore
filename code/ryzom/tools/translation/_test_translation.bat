cd bin

del log.log
input_output_service_rd.exe -Q

IF EXIST \\server\code\tools\log_analyser\log_analyser.exe (
	start \\server\code\tools\log_analyser\log_analyser.exe log.log
) ELSE (
	start notepad.exe log.log
)

cd ..
