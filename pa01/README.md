# PA 01 - Shell State & Composition

## Part 1 - Names, Commands, and Process State

### 1.1

**Prediction**

pwd will print the directories inbox is nested within

navigating to a sibling directory requires first backing into the parents of the two side-by-side directories

**Observation**

```
$ pwd
/home/goliasa1/Documents/UNIX/CPS_444/pa01/workspace/inbox

$ cd ../archive/
goliasa1@student-virtual-machine-24:~/Documents/UNIX/CPS_444/pa01/workspace/archive
```

**Explanation**

pwd - prints working directory
This command shows all parent directories of the one currently being worked within. Thus the absolute pathname is:
/home/goliasa1/Documents/UNIX/CPS_444/pa01/workspace/inbox/tasks.txt

Since inbox/ and archive/ are both children of workspace/, to get from inbox/ to archive/ you must first back up to the parent directory (using ..) and then enter into archive/


### 1.2

**Prediction**

copying the file requires taking the file as an argument, and then specifying the sibling directory for the copied file location

**Observation**

```
$ cp tasks.txt ../archive/tasks.copy
$ ls ../archive/
tasks.copy
```

**Explanation**

The cp command copies the first entry (tasks.txt) to the given location (../archive/tasks.copy). In order to move outside of the working directory, .. is used to back into the parent, and then the further directory is specified.

Now workspace/ looks like:
inbox/tasks.txt
archive/tasks.copy

### 1.3

**Prediction**

since the file was created by running ``` $ touch "two words.txt" ``` I predict it will need quotes around the file name in order to interact with it

running ls on inbox/ returns 'two words.txt'

**Observation**

```
$ file 'two words.txt'
two words.txt: ASCII text

$ head -3 'two words.txt'
This is the head

course=cps444
```

**Explanation**

The file name contains a space, making it treated like a string when accessing it via the terminal. The absolute pathname of the file is:
/home/goliasa1/Documents/UNIX/CPS_444/pa01/workspace/inbox/two words.txt

Because the file is short, I chose to only pick up the first 3 lines of the file to point out the difference from running the cat command on the file.

### 1.4

**Prediction**

cd should be a built-in shell command

ls should be a built-in shell command, but the -P be an input such that the command is external to shell

**Observation**

```
$ type cd
cd is a shell builtin

type -P ls
/usr/bin/ls
```

**Explanation**

cd is a part of the shell commands. It is necessary for changing directories and navigating to subdirectories

ls is an external program. The command searches through the executable path and report the pathname of ls, which is in the user bin

Shell builtins can read/write the state of the current process, while external commands execute as a separate process

### 1.5

**Prediction**

shell processes should persist through new processes

other processes can be killed within different windows

**Observation**

```
$ ps -o pid,ppid,comm
    PID    PPID COMMAND
 106334   14222 bash
 108951  106334 ps
```
New window:
```
 $ ps -o pid,ppid,comm
    PID    PPID COMMAND
 108975   14222 bash
 108984  108975 ps
```

**Explanation**

The parent process ID of the ps command is that of the bash command's process ID. This is because the inspection process is a child of the bash process.

When I opened a new window and reran the command, the bash parent process ID remained the same, however, the ps PPID changed. In both cases, the process ID changed.
