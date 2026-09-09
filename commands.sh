course=cps444
unit=03
printf "${course}-unit-${unit}\n"

printf '\n'
name='Andrew Golias'
printf 'Regular Date\n'
printf '<%s>\n' "$(date)"
printf 'Separated Date\n'
printf '<%s>\n' $(date)
printf '\n'
printf '<%s>\n' "${name}"
printf '|%s|\n' $name

oof=
printf '\n%s\n' '${oof}'

printf '\n'
pattern='*.txt'
printf '<%s>\n' $pattern
