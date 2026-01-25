cls
call gcc -Wall -Wextra -ggdb -o0 -o main.exe main.c
@echo off
IF /I "%1" == "run" (
	@echo on
	call main.exe %2 %3
)
@echo off
IF /I "%1" == "debug" (
	@echo on
	call wsl gcc -Wall -Wextra -ggdb -o main main.c
	call wsl valgrind ./main %2 %3 --leak-check=full
	call del main
)

@echo off
IF /I "%1" == "gdb" (
	@echo on
	call wsl gcc -Wall -Wextra -ggdb -o main main.c
	call wsl gdb main
	call del main
)