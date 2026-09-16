# PA 01 - Shell State & Composition

## Part 1 - Names, Commands, and Process State

### 1.1

**Prediction**

pwd will print the parent directories inbox is within

navigating to a sibling directory requires first backing into the parents of the two side-by-side directories

**Observation**

```text
$ pwd
/home/goliasa1/Documents/UNIX/CPS_444/pa01/workspace/inbox

$ cd ../archive/
```

**Explanation**

A pathname is used to locate an object within the system. Relative pathnames start the search at the current working directory (pa01/workspace/inbox/). Absolute pathnames give the ability to traverse from root.

pwd - prints working directory
This command shows all parent directories of the one currently being worked within. Thus the absolute pathname is:
/home/goliasa1/Documents/UNIX/CPS_444/pa01/workspace/inbox/tasks.txt

Since inbox/ and archive/ are both children of workspace/, to get from inbox/ to archive/ you must first back up to the parent directory (using ..) and then enter into archive/

The relative pathname from inbox/ to archive/ is ../archive/

Navigating to archive changes the tag to:
goliasa1@student-virtual-machine-24:~/Documents/UNIX/CPS_444/pa01/workspace/archive $



### 1.2

**Prediction**

copying the file requires taking the file as an argument, and then specifying the sibling directory for the copied file location

**Observation**

```text
$ cp tasks.txt ../archive/tasks.copy
$ ls ../archive/
tasks.copy
$ ls -l tasks.txt
-rw-rw-r-- 1 goliasa1 goliasa1 0 Sep 14 18:37 tasks.txt
$ ls -l ../archive/tasks.copy 
-rw-rw-r-- 1 goliasa1 goliasa1 0 Sep 14 18:52 ../archive/tasks.copy
```

**Explanation**

The cp command copies the contents of the source file into a new file given its destination location:
+ Source: tasks.txt
+ Destination: ../archive/tasks.copy

This leaves the tasks.txt untouched and a copy of its data in tasks.copy. The metadata from the ls -l command proves the copy contains the same state, as its permissions match among the user, groups, and others.

In order to move outside of the working directory, .. is used to back into the parent, and then the further directory is specified.

Now workspace/ looks like:
+ /inbox/tasks.txt - state is unchanged
+ /archive/tasks.copy - state is updated with copied file

### 1.3

**Prediction**

since the file was created by running ``` $ touch "two words.txt" ``` I predict it will need quotes around the file name in order to interact with it

running ls on inbox/ returns 'two words.txt'

**Observation**

```text
$ file 'two words.txt'
two words.txt: ASCII text

$ head -3 'two words.txt'
This is the head

course=cps444
```

**Explanation**

The file name contains a space, making it treated like a string to prevent word splitting, and must be noted as a single word, when accessing it via the terminal. The absolute pathname of the file is:
/home/goliasa1/Documents/UNIX/CPS_444/pa01/workspace/inbox/two words.txt

The filename no longer includes quotes because they act as a signal to bash to not perform splitting at a whitespace boundary. This is called quote removal and it is a shell instruction that tells the shell how to interpreter the filename.

Because the file is short, I chose to only pick up the first 3 lines of the file to point out the difference from running the cat command on the file.

### 1.4

**Prediction**

cd should be a built-in shell command

ls is not a built-in shell command and -P defines search for a PATH

**Observation**

```text
$ type cd
cd is a shell builtin

type -P ls
/usr/bin/ls
```

**Explanation**

cd is a part of the shell commands. It is necessary for changing directories and navigating to subdirectories, and it is an attribute of the process's state

ls is an external program. The command searches through the executable path and report the pathname of ls, which is in usr/bin

Shell builtins can read/write the state of the current process, while external commands execute as a separate process

### 1.5

**Prediction**

shell processes should persist through new processes

other processes can be killed within different windows

**Observation**

```text
$ ps -o pid,ppid,comm
    PID    PPID COMMAND
 106334   14222 bash
 108951  106334 ps
```
New window:
```text
 $ ps -o pid,ppid,comm
    PID    PPID COMMAND
 108975   14222 bash
 108984  108975 ps
```

**Explanation**

base PID = 106334

ps PID = 108951 PPID = 106334

The parent process ID of the ps command is that of the bash command's process ID. This is because the inspection process is a child of the bash process, the parent process. This shows that the bash process is the one that launched the ps process, meaning there is a parent/child relationship.

When I opened a new window and reran the command, the bash parent process ID remained the same, however, the ps PPID changed. In both cases, the process ID changed. Also, the PID itself updated, proving that new terminal windows create a new parent bash process.



## Part 2 - Read & Change Permissions

### 2.1

**Prediction**

the permission of the file should be under my username with root access

the default mode before changing to 640 should be read/write for the user (file owner) and group with no general other permissions as the file was created under my personal user, which has root privileges

**Observation**

```text
$ id -un
goliasa1

$ id -Gn
goliasa1 sudo users

$ ls -l run.sh 
-rw-r----- 1 goliasa1 goliasa1 0 Sep 15 13:28 run.sh
```

**Explanation**

The first id command lists my username, goliasa1

The second id command lists the groups associated with my username, sudo & users, implying my user has root access

The long-listing from the ls command shows my user as the one who created the run.sh file and my group, which is also happens to be my username

When attempting to access the file, Linux will check the first triplet, which is -rw. As I am the user trying to access this file, it grants permission to read and write to the file. If accessing under a different user, Linux would next check the group

### 2.2

**Prediction**

putting the script in mode 640 should keep read/write permissions for the file owner (my user), limit group permissions to read, and restrict all permissions from others

**Observation**

```text
$ chmod 640 run.sh
$ ls -l run.sh 
-rw-r----- 1 goliasa1 goliasa1 0 Sep 15 17:06 run.sh
```

**Explanation**

As predicted, the owner/group/other triplets align in the output of the ls -l command. The file owner can read & write, groups can read, other users have no access to the file

There is an initial decode bit is - since it is a file, not a directory

+ the first triplet: 6 decodes to 110 (rw-)
+ the second triplet: 4 decodes to 100 (r--)
+ the third triplet: 0 decodes to 000 (---)

### 2.3

**Prediction**

u+x adds execute permission to the user who created the file

the other octets are not specifically updated, meaning no other changes will occur

**Observation**
```text
chmod u+x run.sh
$ ls -l run.sh 
-rwxr----- 1 goliasa1 goliasa1 0 Sep 15 17:06 run.sh
```

**Explanation**

Now, run.sh is outlined in green, implying it has been upgraded to an executable file. When running the ls -l command, the only change from 2.2 is that the file owner is rwx.

Before this command, ```./run.sh``` would have returned a permission error. However, now ```./run.sh``` is executed as a blank scipt.

### 2.4

**Explanation**

Execute on a file means that the kernel can run the file directly as a program or script process. This is possible as a file simply contains scripting code that can be executed.

Execute on a directory means that a process can traverse and search names within that directory, or perform the cd command. Directories contain files, but do not directly have scripts stored at that level. This means that it does not make sense to simply run a directory, rather a process can cd through a directory to find something within one of its children files/directories.


## Part 3 - Expansion Produces Argument Words

### 3.1

**Prediction**

output should be: ```<two words>```

**Observation**
```text
$ printf '<%s>\n' $label
<two>
<words>
```

**Explanation**

This is an example of shell performing parameter expansion. This means it is substituting $label with its value, two words. Bash then performs word splitting by using whitespace to separate two and words before passing it to the printf function.

This differed from my prediction as I forgot whitespace becomes relevant when using unquoted variables

### 3.2

**Prediction**

output should be: ```<two words>```

**Observation**
```text
$ printf '<%s>\n' "$label"
<two words>
```

**Explanation**

This is an example of parameter expansion that replaces $label with its value, two words. Since the variable is within double quotes, word splitting does not occur and when quote removal occurs, the entire value is passed to the printf function.

### 3.3

**Prediction**

output should be: ```<$label>```

**Observation**
```text
$ printf '<%s>\n' '$label'
<$label>
```

**Explanation**

Single quoting prevents any type of expansion. Therefore, after quote removal, the literal $label is passed to the printf function.

### 3.4

**Prediction**

output should be:
```text
<tasks.txt>
<two.txt>
<events.txt>
```

**Observation**
```text
$ printf '<%s>\n' $pattern
<events.txt>
<tasks.txt>
<two words.txt>
```

**Explanation**

This is an example of parameter expansion as $pattern is replaced first with *.txt. The wildcard triggers pathname expansion of all .txt files to be passed as arguments to the printf function

### 3.5

**Prediction**

output should be: ```<*.txt>```

**Observation**
```text
$ printf '<%s>\n' "$pattern"
<*.txt>
```

**Explanation**

This is an example of parameter expansion where double quoting prevents pathname expansion. Since the pathname cannot be expanded using the wildcard, as in the previous printf, the literal *.txt is passed to the printf function.

### 3.6

**Prediction**

output should be:
```text
<red>
<green>
<>
```

**Observation**
```text
$ printf '<%s>\n' $(printf 'red green\n')
<red>
<green>
```

**Explanation**

Parameter expansion in the unquoted form allows the shell to perform word splitting on the string to produce two arguments that are given to the outer printf function.

I was wrong in predicting that the new line would generate a third, empty argument as the trailing \n is removed in the subshell command.

### 3.7

**Prediction**

output should be: ```<red green>```

**Observation**
```text
$ printf '<%s>\n' "$(printf 'red green\n')"
<red green>
```

**Explanation**

The subshell command prevent word splitting on the returned sting, meaning that the outer printf function receives only one argument.

### 3.8

**Prediction**

output should have 7 values and be:
```text
<alpha>
<beta>
<events.txt>
<two words.txt>
<tasks.txt>
<alpha beta>
<*.txt>
```

**Observation**
```text
$ printf '<%s>\n' $words $pattern "$words" "$pattern"
<alpha>
<beta>
<events.txt>
<tasks.txt>
<two words.txt>
<alpha beta>
<*.txt>
```

**Explanation**

+ $words - parameter expansion divides the string by word splitting
+ $pattern - *.txt undergoes pathname expansion to match the wildcard to its filename arguments
+ "$words" - double quoting prevents word splitting, passing the literal value
+ "$pattern" - double quoting prevents matching and pathname expansion, leaving the literal *.txt to be passed

There are 7 values, as predicted, passed to the printf function


## Part 4 - Rewire Streams, Then Compose Programs

### 4.1

**Prediction**

the contents of tasks.txt will be printed to the terminal and copied into archive/cat.out

the error code that missing.txt is not a file or directory will be copied into archive/cat.err

+ desc 1 (stdin) - printed to the terminal
+ desc 2 (stdout) - forwarded by > to cat.out
+ desc 3 (stderr) - forwarded by 2> to cat.err

**Observation**
```text
$ cat tasks.txt missing.txt > ../archive/cat.out 2> ../archive/cat.err
$ cat ../archive/cat.out
TODO: quote variables
DONE: inspect paths
TODO: test pipeline
$ cat ../archive/cat.err
cat: missing.txt: No such file or directory
```

**Explanation**

The bytes from the valid cat command run on tasks.txt went to desciptor 1. Forwarding the standard output to the .out file

The bytes from the invalid cat command run on missing.txt went to descriptor 2, because of the 2> operator. Forwarding the standard error to the .err file 

### 4.2

**Prediction**

the order of redirection matters because in the second command, the operator will causes the error log to be printed to the terminal and not necessarily just to the files

**Observation**
```text
$ cat tasks.txt missing.txt > ../archive/all.txt 2>&1
$ cat ../archive/all.txt 
TODO: quote variables
DONE: inspect paths
TODO: test pipeline
cat: missing.txt: No such file or directory

$ cat tasks.txt missing.txt 2>&1 > ../archive/out-only.txt
cat: missing.txt: No such file or directory
```

**Explanation**

Redirection operators are evaluated left to right

The first command shows the standard output and error operators both at the end, causing both messages to be forwarded to the all.txt file

The second command shows the operators before the redirect file, only changing the standard output and leaving the standard error message in the terminal

This happens because the duplication must be the last part of the command in order to redirect the messages to a file aleady specified in the command

### 4.3

**Prediction**

each unique event will be printed, prefixed by the number of times it appears in the file, -nr will likely then sort by number order

**Observation**
```text
$ cat events.txt 
warning
error
info
warning
error
warning
info
error
error

$ cat events.txt | sort
error
error
error
error
info
info
warning
warning
warning

$ cat events.txt | sort | uniq -c
      4 error
      2 info
      3 warning

$ cat events.txt | sort | uniq -c | sort -nr
      4 error
      3 warning
      2 info

$ cat events.txt | sort | uniq -c | sort -nr > ../archive/event-counts.txt
```

**Explanation**

Sort must appear before uniq because uniq only compares adjacent items, rather than traversing then entire list when checking for uniqueness. This simplifies the function as it can be assumed that the list is already sorted and then only check if the next output has just recently been printed to find and count unique outputs

The full command ends up printing all messages from events.txt only once, with the count of how many times they appear, and messages are sorted in descending order by this count

### 4.4

**Observation**
```text
$ cat tasks.txt not-there.txt 2> ../archive/synthesis.err | grep TODO | sort > ../archive/synthesis.out

$ cat ../archive/synthesis.err
cat: not-there.txt: No such file or directory
$ cat ../archive/synthesis.out
TODO: quote variables
TODO: test pipeline
```

+ cat: stdin - terminal; stdout - pipe 1; stderr - ../archive/synthesis.err
+ grep TODO: stdin - pipe 1; stdout - pipe 2; stderr - terminal
+ sort: stdin - pipe 2; stdout - ../archive/synthesis.out; stderr - terminal

**Explanation**

cat attempts to print tasks.txt and not-there.txt

The standard error from the invalid file not-there.txt is written to descriptor 2 and forwarded to the .err file

The standard output from the valid file tasks.txt is piped to grep TODO, which filters all lines containing the substring "TODO", is piped to sort, and then is written to descriptor 1 and forwarded to the .out file


## Part 5 - Data & Exit Status are Different Channels

### 5.1

**Observation**
```text
$ grep -q TODO tasks.txt
$ printf 'TODO status = %s\n' "$?"
TODO status = 0

$ grep -q FIXME tasks.txt
$ printf 'FIXME status = %s\n' "$?"
FIXME status = 1
```

**Explanation**

```$?``` checks the exit status of the previous command run. If grep returns a match, the exit status is stored as 0, otherwise, 1.

The grep command filters the input file, tasks.txt, by the argument TODO (-q quiets any output from appearing in the terminal). The returned status stored in the special shell parameter is 0, since TODO appears in the file. 


### 5.2

**Prediction**

the pipeline may act like an or gate, both commands should return 0

**Observation**
```text
$ false | true
$ printf 'pipeline status = %s\n' "$?"
pipeline status = 0

$ true | false
$ printf 'pipeline status = %s\n' "$?"
pipeline status = 1
```

**Explanation**

Bash's exit status of the full pipeline is the result of the final command within that pipeline.

The last command in the first command entered is true, which is why the status returned equals 0.

The last command in the second command entered is false, which is why the status returned equals 1.