# Version_Control_System

## Link
https://github.com/kbbhatt04/Version_Control_System

## Requirements
1. g++ compiler (to compile the code)
2. SHA1.hpp (Imported SHA1 library. File included in the folder)
3. Made for Linux


## How to run : 

### Compile
1. Compile the code using the command : `g++ git.cpp -o git`
   (An executable file with the name 'git' will appear in the current folder)

### To run the program and use the functionalities of git:
2. Init : `./git init` 
    A hidden folder with the name `.git` will be created in the current folder
	
3. Add : `./git add <filename/foldername>`
		Example: ./git add folder1
		*Takes one folder/file at a time
	
4. Status : `./git status`
		*Displays tracked, untracked and modified files
	
5. Commit : `./git commit <mode> <message>`
		Additional modes:
			
			5.1 Commit without a message : `./git commit`
			
			5.2 Adding a message along with the commit (-m) : `./git commit -m <message>`
			      Example: ./git commit -m My first commit
	
6. Rollback : `./git rollback`
		Additional modes:
			
			6.1 Rollback to previous version : `./git rollback`
			
			6.2 Rollback to certain checkpoints back at once (-c) : `./git rollback -c <number of hops>`
			      Example: ./git rollback -c 3 (will go three versions back. Eg: V5 -> V2)
			
			6.3 Rollback to a specific version : `./git rollback -vn V<version number>`
			      Example: ./git rollback -vn V2 (will rollback from current version to Version 2)
			
7. Log : `./git log`
		*Shows version logs
	       
	
