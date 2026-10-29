# PA 03 - UNIX Files and Interfaces in C

## Build and Source Organization

This assignment creates a single filetool executable upon running the Makefile. The organization is as follows:

+ `main.c` parses the command-line arguments provided with the filetool executable, distinguishes between `info` and `copy` commands, prints usage diagnostics for invalid inputs, and returns the status
+ `file_ops.c` implements filesystem metadata inspection with stat or byte copying with open, read, write, and close
+ `file_ops.h` declares public functions, info and copy, which are shared between `main.c` and `file_ops.c`
+ `Makefile` compiles source files separately, links filetool, declares object files as header dependencies upon running `make`; provides `clean` to remove excess files

**Building object and tool files**

```bash
$ make clean
rm -f filetool main.o file_ops.o
$ make all
cc -Wall -Wextra -std=c17 -g -c main.c
cc -Wall -Wextra -std=c17 -g -c file_ops.c
cc -Wall -Wextra -std=c17 -g -o filetool main.o file_ops.o
```

+ Clean base repository and then generate objects and filetool

```bash
$ ./filetool 
usage: ./filetool info PATH
       ./filetool copy SOURCE DEST
$ printf 'status=%s\n' "$?"
status=2
```

+ Prints usage function to stderr to display command-line argument ordering; returns status 2

## Interface and Design

Two command forms:

```bash
./filetool info PATH
```

+ Uses stat function and prints information about file or directory given as input:
    + path (exactly as given from command-line input)
    + type (file/directory/other)
    + size (in bytes)
    + mode (user, group, other permissions)

```bash
./filetool copy SOURCE DEST
```

+ Uses open, read, write, and close functions and gives various formats for copying byte streams from files or stdin to new destinations or stdout
+ An explicit destination file in the copy command must have a unique path. It will not overwrite existing files as expected due to opening with O_WRONLY | O_CREAT | O_EXCL
+ Copy command has the following options for reading and creating/outputting bytes:

| Form | Meaning |
| :--- | :--- |
| copy SRC DST | Read named file; create named file |
| copy SRC - | Read named file; send bytes to  stdout |
| copy - DST | Read stdin; create named file |
| copy - - | Transfer stdin to stdout |

**Exit Statuses**

+ `0` for success
+ `1` for FileSystem or I/O failures, such missing file arguments
+ `2` for invalid command-line usage


## Tests and Results

Assumes pa03-tests/ has the following files as per assignment instructions:
+ binary.bin
+ empty.bin
+ existing-link -> missing-target
+ large.bin
+ tiny-link.txt -> tiny.txt
+ tiny.txt
+ 'two words.txt'

**Usage Tests**

```bash
$ ./filetool 
usage: ./filetool info PATH
       ./filetool copy SOURCE DEST
$ echo $?
2

$ ./filetool info
usage: ./filetool info PATH
       ./filetool copy SOURCE DEST
$ echo $?
2

$ ./filetool info . ..
usage: ./filetool info PATH
       ./filetool copy SOURCE DEST
$ echo $?
2

$ ./filetool copy ../pa03-tests/tiny.txt ../pa03-tests/large.bin ../pa03-tests/large.bin 
usage: ./filetool info PATH
       ./filetool copy SOURCE DEST

$ echo $?
2
```

+ Usage message is displayed when filetool command is unspecified, when `info` is not given exactly one pathname, and when `copy` is not given exactly two pathnames

**Info Tests**

```bash
$ ./filetool info ../pa03-tests/tiny.txt
path=../pa03-tests/tiny.txt
type=regular
size=11
mode=0640
$ echo $?
0

$ ./filetool info ../pa03-tests/
path=../pa03-tests/
type=directory
size=4096
mode=0775
$ echo $?
0

$ ./filetool info ../pa03-tests/tiny-link.txt 
path=../pa03-tests/tiny-link.txt
type=regular
size=11
mode=0640
$ echo $?
0
```

+ Successful inspection of existing file and directory; prints path given, type, size, and permissions; returns status code 0
+ The symbolic link properly returns the same size of the tiny.txt file it points to

```bash
$ ./filetool info ../pa03-tests/missing-target > ../pa03-tests/info.stdout 2> ../pa03-tests/info.stderr
$ echo $?
1
$ cat ../pa03-tests/info.stdout
$ cat ../pa03-tests/info.stderr
../pa03-tests/missing-target: No such file or directory

$ ./filetool info ../pa03-tests/existing-link > ../pa03-tests/info.stdout 2> ../pa03-tests/info.stderr
$ echo $?
1
$ cat ../pa03-tests/info.stdout
$ cat ../pa03-tests/info.stderr
../pa03-tests/existing-link: No such file or directory
```

+ Error messages are forwarded to stderr; the missing-target file pathname does not exist and can not be retrieved by the info command; returns status code 1
+ The existing-link is shown with the `ls` command in the pa03-tests/ folder, however, it is a symbolic link to a file that does not exist; causing a redirect to stderr; returns status code 1

**Copy Tests**

```bash
$ ./filetool copy ../pa03-tests/tiny.txt ../pa03-tests/tiny-copy.txt
$ echo $?
0
$ cat ../pa03-tests/tiny-copy.txt 
alpha
beta
$ cmp -s ../pa03-tests/tiny.txt ../pa03-tests/tiny-copy.txt 
$ echo $?
0

$ ./filetool copy ../pa03-tests/empty.bin ../pa03-tests/empty-copy.bin
$ cmp -s ../pa03-tests/empty.bin ../pa03-tests/empty-copy.bin 
$ echo $?
0

$ ./filetool copy ../pa03-tests/large.bin ../pa03-tests/large-copy.bin
$ cmp -s ../pa03-tests/large.bin ../pa03-tests/large-copy.bin
$ echo $?
0
$ ./filetool info ../pa03-tests/large-copy.bin 
path=../pa03-tests/large-copy.bin
type=regular
size=20000
mode=0644
```

+ Successful copying silently sends the byte stream from the source pathname to the new unique destination pathname; returns status code 0
+ Running `cmp -s` on the files returns 0, showing that the files do not merely contain the same text, but one is in fact a copy of the other
+ Copying does not stop at end of file, inspection on the large file proves it's entire contents are copied as it's copy file also contains 20,000 bytes, despite using a buffer size of 4096

```bash
$ ./filetool copy ../pa03-tests/tiny.txt ../pa03-tests/large.bin 
../pa03-tests/large.bin: File exists
$ echo $?
1
$ ./filetool info ../pa03-tests/large.bin 
path=../pa03-tests/large.bin
type=regular
size=20000
mode=0664

```

+ Existing files are not overwritten, the inspection shows the large file is the size same after the overwrite process is attempted and fails; returns status code 1

**Standard Streams**

```bash
$ ./filetool copy ../pa03-tests/tiny.txt -
alpha
beta
$ echo $?
0

$ ./filetool copy - ../pa03-tests/stdin.txt
Input from command line
$ cat ../pa03-tests/stdin.txt 
Input from command line

$ ./filetool copy - -
im seeing double
im seeing double
hello
hello
```

+ The `-` allows a pathname to remain unspecified, as specified above, the position of the dash tells the filetool whether standard input or output is used; returns status code 1

```bash
$ printf 'abc\n' | ./filetool copy - - | wc -c
4

$ ./filetool copy ../pa03-tests/tiny.txt - | wc -c
11
```

+ The first example pipes the print statement to stdin, then runs the `copy` command on stdin and copies the result of the last pipe (character count) to stdout
+ The second example copies the tiny.txt file to stdout and then the pipe prints the character count; without this pipe, the contents of the file are simply printed


## Reflection

1. Why is read() == 0 different from read() == -1?

read() returning `0` means end-of-file has been reached and there are no more bytes to read from the stream. This is the stopping condition for the loop that stops copying bytes. Returning a positive value tells the copy loop how many bytes were read from the file. Returning -1 here would imply an incomplete file read and cancel the copy operation unnecessarily.

read() returning `-1` means the operation failed, which is treated as an I/O failure. Diagnostics are printed to stderr and returns the status code 1. This stops the copy loop from continuing as soon as an error occurs. Returning 0 here would mask errors and cause failed copy attempts to pass through.


2. Why can `./filetool copy input.bin -` be used in a shell pipeline without the program knowing which command comes next?

When `-` is the destination, the filetool writes the copied byte stream to the file descriptor 1, which is stdout, rather than a named output file pathname. The shells sets up a pipeline by connecting one command's stdout to the next command's stdin before running the commands. This means that filetool only needs to write to desc 1, is does not necessarily need to know which command will be using its input. This can be show in the following example, where tiny.txt is copied to stdout, read in from stdin, and then copied again to stdout

```bash
$ ./filetool copy ../pa03-tests/tiny.txt - | ./filetool copy - -
alpha
beta
```

Diagnostics and usage messages go to desc 2, stderr, so they do not become part of the byte stream sent through stdout. This is shown as the stderr message is a nonzero size of bytes, but the returned character count remains as 0:

```bash
$ set -o pipefail
$ ./filetool | wc -c
usage: ./filetool info PATH
       ./filetool copy SOURCE DEST
0
$ echo ${PIPESTATUS[@]}
2 0
```