cls

@echo off
IF /I "%1" == "" (
	@echo on
	call gcc -Wall -Wextra -ggdb -o0 -o main.exe main.c
)

@echo off
IF /I "%1" == "run" (
	@echo on
	call gcc -Wall -Wextra -ggdb -o3 -o main.exe main.c
	call main.exe %2 %3 %4
)
@echo off
IF /I "%1" == "debug" (
	@echo on
	call wsl gcc -Wall -Wextra -o0 -ggdb -o main main.c
	call wsl valgrind ./main %2 %3 %4 --leak-check=full
	call del main 
)

@echo off
IF /I "%1" == "gdb" (
	@echo on
	call wsl gcc -Wall -Wextra -o0 -ggdb -o main main.c
	call wsl rlwrap --always-readline gdb main
	call del main
)