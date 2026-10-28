#!/usr/bin/env bash
set -u
set -o pipefail

## standardizes sort/uniq stage for reporting pipeline
export LC_ALL=C

tmp_file=""

## returns print statement when script is called with malformed parameters
usage() {
	printf 'usage: %s OUTPUT_FILE INPUT_FILE...\n' "$0" >&2
}

## cleanup and remove temporary file
cleanup() {
	if [[ -n "$tmp_file" && -e "$tmp_file" ]]; then
		rm -f -- "$tmp_file"
	fi
	tmp_file=""
}

handle_int() {
	exit 130
}

handle_term() {
	exit 143
}	

# helper functions

## returns true if REPORT_LEVELS is a syntactically valid regular expression because this variable can be set upon calling the script
is_valid_regex() {
	local pattern=$1
	awk -v p="$pattern" 'BEGIN { discard = ("" ~ p) }' </dev/null 2>/dev/null
}

## returns true if value matches valid pattern, false if value is negative
is_nonnegative_integer() {
	local val=$1
	[[ "$val" =~ ^[0-9]+$ ]]
}

## validate all input files and their records
is_valid_record() {
	local line=$1
	local field='[a-zA-Z0-9._-]+'
	[[ "$line" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}[[:space:]]+(INFO|WARN|ERROR)([[:space:]]+${field}){3}$ ]]
}

validate_input_files() {
	local file
	for file in "$@"; do
		if [[ ! -f "$file" ]]; then
			printf 'error: input file missing or not regular file: %s\n' "$file" >&2
			return 3
		fi
		if [[ ! -r "$file" ]]; then
			printf 'error: input file is unreadable: %s\n' "$file" >&2
			return 3
		fi
		local line
		while IFS= read -r line || [[ -n "$line" ]]; do
			if ! is_valid_record "$line"; then
				printf 'error: malformed record in %s: %s\n' "$file" "$line" >&2
				return 3
			fi
		done < "$file"
	done
	return 0
}

## read records on stdin, ranked by count then label
summarize_field() {
	local field=$1
	awk -v f="$field" ' { print $f }' \
	| sort \
	| uniq -c \
	| awk '{ printf "%s %s\n", $1, $2 }' \
	|sort -k1,1nr -k2,2
}

## select records whose level field matches the configured regular expression
select_records() {
	local levels_regex=$1
	shift
	cat -- "$@" | awk -v re="^(${levels_regex})\$" '$2 ~ re'
}

## create and publish report
build_report() {
	local levels_regex=$1
	local n_inputs=$2
	shift 2

	local records
	if ! records=$(select_records "$levels_regex" "$@"); then
		printf 'error: record selection failed\n' >&2
		return 4
	fi
	
	local total=0
	if [[ -n "$records" ]]; then
		if ! total=$(printf '%s\n' "$records" | wc -l); then
			printf 'error: record count failed\n' >&2
			return 4
		fi
		total=${total//[[:space:]]/}
	fi

	printf 'Event Report\n'
	printf '============\n'
	printf 'Selected levels: %s\n' "$levels_regex"
	printf 'Input files: %d\n' "$n_inputs"
	printf 'Matching records: %d\n\n' "$total"

	printf 'By Level\n'
	printf '%s\n' "--------"
	if [[ "$total" -gt 0 ]]; then
		printf '%s\n' "$records" | summarize_field 2 || return 4
	fi
	printf '\n'

	printf 'By Service\n'
	printf '%s\n' "----------"
	if [[ "$total" -gt 0 ]]; then
		printf '%s\n' "$records" | summarize_field 3 || return 4
	fi
	printf '\n'

	printf 'Selected Events\n'
	printf '%s\n' "---------------"
	if [[ "$total" -gt 0 ]]; then
		printf '%s\n' "$records"
	fi
	return 0
}

## execute main functions and trap exit statuses
main() {
	trap cleanup EXIT
	trap handle_int INT
	trap handle_term TERM

	# validate parameters
	if [[ "$#" -lt 2 ]]; then
		usage
		return 2
	fi

	local output=$1
	shift

	# check directory location of output file & redirect stderr
	if [[ -d "$output" ]]; then
		printf 'error: output pathname is a directory: %s\n' "$output" >&2
		return 2
	fi

	local output_dir=$(dirname -- "$output")
	if [[ ! -d "$output_dir" ]]; then
		printf 'error: output directory does not exist: %s\n' "$output_dir" >&2
		return 4
	fi

	# validate envt config
	local report_levels="${REPORT_LEVELS:-WARN|ERROR}"
	if ! is_valid_regex "$report_levels"; then
		printf 'error: REPORT_LEVELS is not a valid regular expression: %s\n' "$report_levels" >&2
		return 2
	fi


	local report_delay="${REPORT_DELAY:-0}"
	if ! is_nonnegative_integer "$report_delay"; then
		printf 'error: REPORT_DELAY must be a nonnegative integer: %s\n' "$report_delay" >&2
		return 2
	fi


	# validate inputs & records
	validate_input_files "$@" || return $?

	# setup temp file & traps
	tmp_file=$(mktemp "${output_dir}/tmp.event-report.XXXXXX") || {
		printf 'error: failed to create temporary file\n' >&2
		return 4
	}

	if ! build_report "$report_levels" "$#" "$@" > "$tmp_file"; then
		printf 'error: failed to generate report\n' >&2
		return 4
	fi

	# testing hook delay
	if [[ "$report_delay" -gt 0 ]]; then
		sleep "$report_delay" & 
		wait $! || true
	fi

	# publish report
	if ! mv -- "$tmp_file" "$output"; then
		printf 'error: failed to publish final report to %s\n' "$output" >&2
		return 4
	fi

	# clear tmp_file to avoid delete report
	tmp_file=""

	printf 'report written: %s\n' "$output"
	return 0

}

main "$@"
