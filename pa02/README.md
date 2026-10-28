# PA 02 - Shell Programming & Automation

## Interface

### Command syntax

Executing `event-report.sh` requires at least 2 arguments: an output file and valid input data files. 

`./event-report.sh <output-file> <input-file>*`

+ output-file: pathname where the report will be published
+ input-file: one or more pathnames to input event-log files for processings
	+ Pathnames containing spaces are supported

### Env variables

+ REPORT_LEVELS: extended regular expression applied to match the LEVEL field, categorized event-log types. Defaults to WARN|ERROR if unset

`REPORT_LEVELS='INFO' ./event-report.sh <output-file> <input-file>`

+ REPORT_DELAY: Testing hook that requires a nonnegative integer for delay in seconds. When delay is greater than zero, the script sleeps after creating the tmp_file and passes the cleanup trap, waiting to publish the report. Defaults to 0.

### Exit Statuses

+ 0: Successful execution and publication of the report, regardless of content within the report.
+ 2: Usage error from command line interface. This can be fewer than 2 file arguments or an unreadable input file. Invalid environment variables
+ 3: Missing, unreadable, or malformed input file
+ 4: Temp file creation or report publication failure
+ 130: Interrupted by SIGINT. This can occur from running the script with a REPORT_DELAY and doing Ctrl+C in the terminal while the process is running.
+ 143: Terminated by SIGTERM. This can occur from killing a process while it is running.


## Design

major functions and responsibility of each

+ main "$@": top-level entry point for the script. The main function validates parameters, evaluates environment variables, sets up trap signal handlers at the beginning of the function, calls helpers functions
+ usage: prints expected valid syntax to command line and stderr. Returns exit status 1
+ cleanup: safely removes tmp_file to remove the temporary state whether the script succeeds, fails, or is interrupted
+ validate_input_files: iterate over each line of all input log files to verify records consist of 5 valid whitespace deliminated fields `DATA LEVEL SERVICE ACTOR EVENT`. Aborts script with exit status 3 and sends logs to stderr upon malformed line and prints error to command line
+ build_report: processes input records against REPORT_LEVELS pattern, formats output into tmp_file to be published
+ summarize_field: code handler for awk command, simplifies layout by handling sorting and counting from a function call, ranks rows by descending count & alphabetical label


## Pipeline

how records become the required report sections

1. `select_records` combines input files and filters records using `cat` and `awk` commands to match LEVEL field against the REPORT_LEVELS env variable
2. **Event Report**:prints the regex, counts the input files, and records files matching the selected level
3. **By Level**: extracts LEVEL from matching records, pipes to `sort | uniq -c` in summarize_field function
4. **By Service**: extracts SERVICE from matching records, pipes to `sort | uniq -c` in summarize_field function
5. **Selected Events**: prints raw matching log entries from their input files


## Robustness

temporary-file ownership, traps, and safe publication

+ Script creates a temporary file using `mktemp` during execution which is removed after the official report is successfully published or if any error prompts the cleanup function. The official report is created by moving the temp file to the output file
+ If validation or report generation fails, the old report is unchanged, this prevents overriding report logs
+ Traps at the beginning of main, the initial entry point to the script, trigger proper exit statuses. These call the cleanup function which removes the temporary file to prevent it from being listed in the repository or overriding reports with bad logs

## Automation

what `cron.example` assumes

Cron jobs are runs from the home directory, and must be provided the absolute pathname for files within its execution. Environment variables must be passed in the command string as cron has limited knowledge of these variables without explicit definition. These tasks are run without an interactive terminal, meaning stdout and stderr must be redirected to capture logs and error diagnostics. This is done with `>> .log 2>> .err` appending logs everyday that the job is executed.

## Testing

a compact table or list covering Tests A-F and additional edge cases I used 

#### Test A - default successful report

```
$ ./event-report.sh reports/report.txt test-data/normal.log test-data/extra.log 
report written: reports/report.txt
$ printf 'status=%s\n' "$?"
status=0
$ cat reports/report.txt 
Event Report
============
Selected levels: WARN|ERROR
Input files: 2
Matching records: 7

By Level
--------
4 ERROR
3 WARN

By Service
----------
3 auth
2 disk
2 web

Selected Events
---------------
2026-09-27 WARN disk server1 usage-91
2026-09-27 ERROR web bob request-timeout
2026-09-27 ERROR auth carol login-denied
2026-09-27 WARN disk server2 usage-87
2026-09-27 ERROR auth erin login-denied
2026-09-27 WARN web node1 latency-high
2026-09-27 ERROR auth frank login-denied

```

#### Test B - Environment override

```
$ REPORT_LEVELS='ERROR' ./event-report.sh reports/errors.txt test-data/normal.log test-data/extra.log 
report written: reports/errors.txt
$ printf 'status=%s\n' "$?"
status=0
$ cat reports/errors.txt 
Event Report
============
Selected levels: ERROR
Input files: 2
Matching records: 4

By Level
--------
4 ERROR

By Service
----------
3 auth
1 web

Selected Events
---------------
2026-09-27 ERROR web bob request-timeout
2026-09-27 ERROR auth carol login-denied
2026-09-27 ERROR auth erin login-denied
2026-09-27 ERROR auth frank login-denied
```

#### Test C - Valid zero-match input

```
$ ./event-report.sh reports/empty.txt test-data/no-alerts.log 
report written: reports/empty.txt
$ printf 'status=%s\n' "$?"
status=0
$ cat reports/empty.txt 
Event Report
============
Selected levels: WARN|ERROR
Input files: 1
Matching records: 0

By Level
--------

By Service
----------

Selected Events
---------------

```

#### Test D - Failed input preserves the old report

```
$ printf 'OLD REPORT\n' > reports/protected.txt 
$ ./event-report.sh reports/protected.txt test-data/malformed.log 
error: malformed record in test-data/malformed.log: this is not a valid event record
$ printf 'status=%s\n' "$?"
status=3
$ cat reports/protected.txt 
OLD REPORT
```

#### Test E - Signal cleanup

+ Test SIGTERM

```
$ REPORT_DELAY=50 ./event-report.sh reports/signal.txt test-data/normal.log &
[2] 351964
$ pid=$!
$ sleep 1
$ kill -TERM "$pid"
$ wait "$pid"
[2]+  Exit 143                REPORT_DELAY=50 ./event-report.sh reports/signal.txt test-data/normal.log
$ printf 'status=%s\n' "$?"
status=143
$ cat reports/signal.txt 
OLD REPORT
```

+ Test SIGINT

```
$ REPORT_DELAY=3 ./event-report.sh reports/signal.txt test-data/normal.log
^C
$ printf 'status=%s\n' "$?"
status=130
```

#### Test F - Pathnames containing spaces

```
$ cp test-data/normal.log 'test-data/space case.log'
$ ./event-report.sh 'reports/report with spaces.txt' 'test-data/space case.log' 
report written: reports/report with spaces.txt
$ printf 'status=%s\n' "$?"
status=0
$ cat 'reports/report with spaces.txt' 
Event Report
============
Selected levels: WARN|ERROR
Input files: 1
Matching records: 5

By Level
--------
3 ERROR
2 WARN

By Service
----------
2 auth
2 disk
1 web

Selected Events
---------------
2026-09-27 WARN disk server1 usage-91
2026-09-27 ERROR web bob request-timeout
2026-09-27 ERROR auth carol login-denied
2026-09-27 WARN disk server2 usage-87
2026-09-27 ERROR auth erin login-denied
$ rm -f 'test-data/space case.log' 
$ ls test-data/
extra.log  malformed.log  no-alerts.log  normal.log
```

#### Edge Cases

+ Usage function hit

```
$ ./event-report.sh reports/report.txt 
usage: ./event-report.sh OUTPUT_FILE INPUT_FILE...
$ printf 'status=%s\n' "$?"
status=2
$ cat reports/report.txt 
Event Report
...
```

> The missing parameter causes the trap to execute the usage function, outputting the expected parameters for this script. Nothing occurs in the script after it recognizes it does not receive an input file and gracefully exits. Status 2 is correctly returned 

+ Missing input file

```
$ ./event-report.sh reports/report.txt test-data/missing.txt
error: input file missing or not regular file: test-data/missing.txt
$ printf 'status=%s\n' "$?"
status=3
$ cat reports/report.txt 
Event Report
...
```

> The missing file cannot be read as input, redirecting stderr to the terminal to display file's contents do not exist. Running `cat` on the output file shows the last report written to the file. Status 3 is correctly returned

+ Invalid env variable

```
$ REPORT_DELAY=-1 ./event-report.sh reports/report.txt test-data/normal.log 
error: REPORT_DELAY must be a nonnegative integer: -1
$ printf 'status=%s\n' "$?"
status=2
$ REPORT_LEVELS='~ @%&*($)%Q(()%#++_R^INFO' ./event-report.sh reports/report.txt test-data/normal.log 
error: REPORT_LEVELS is not a valid regular expression: ~ @%&*($)%Q(()%#++_R^INFO
$ printf 'status=%s\n' "$?"
status=2
```

> The negative REPORT_DELAY and invalid REPORT_LEVELS both cause the script to not execute as delay must be nonnegative and levels must match the defined extended regular expression. In both cases, the script exits and reports the error to the terminal. Status 2 is correctly returned

