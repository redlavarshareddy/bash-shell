I have implemented all the features

# Compile
```make	```

# Run
```./shell```

# Re-Compile
```make clean```
followed by
```make```
# Description
This is a shell.
Code is divided as follows:
## shell.h
Contains all the common header files and common data used by functions
## prompt.c
Contains ```main()```
## built_in.c
Contains the basic functions of the shell:
1. pwd
2. cd
3. echo

## ls.c
Contains ls command
## time.c
Contains time related commands:
1. ```clock```
2. ```remindme```

## execute.c
Contains code for spawning and monitoring child processes.
1. Executing and invoking other programs from command line
2. ```pinfo``` command

## user_details.c
Contains the functions required to fetch user data from ```/etc/passwd``` file.
Eg. username etc.
