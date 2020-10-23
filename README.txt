Author: Michael White mjwh236@uky.edu 04/24/2020
Contents:
    shell.c: implementation of shell
    shell.h: header file for shell
    parser.c: parses commands
    Makefile: builds the program
    README.txt: this file

Running:
    To build shell:
    make clean
    make

    To run with script input:
    ./shell [<scriptfile>]

    To run with prompt
    ./shell

Implementation notes:
    I seperated the shell into three primary 
    functions. The main function is where The
    shell will loop until exited. The 
    control_logic function will check the 
    args[0] of the command to see if the command
    is built in to the shell. If not implemented
    the control_logic function will call the 
    call_redirected function to redirect to a 
    system program. The function call_redirected
    is adapted from the I/O lab.
Limitations:
    The command line only supports a buffer
    of 512 characters. 
    I couldn't figure out how to use stdin as an
    input to a file so in main there is one while
    loop for scripts and a seperate loop for 
    user inputs.
    Commands only support up to 30 arguments due
    to parser.c.


References:
    Adapted a line from cboard.cprogramming.com
    by Lucas Pewkas to recieve imput in a while 
    loop using fgets
    https://cboard.cprogramming.com/c-programming/65068-fgets-while-loop.html
