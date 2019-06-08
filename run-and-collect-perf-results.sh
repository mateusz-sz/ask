#!/bin/bash

# WARNING: this does not follow KISS unfortunately

# echo to STDERR (is run in a subshell to avoid redirections conflicts)
errorecho(){ >&2 echo $@; }

trap 'exit 1' SIGINT

if [[ "$#" -lt 5 ]]
then
    errorecho "Less than 5 arguments given."
    exit 1
fi
SILENT=$1
if [[ ! $SILENT || ( $SILENT -ne 1 ) ]]
then
    errorecho "This is an perf.sh helper. It is intended to run perf.sh with given parameters for specified number of times and write the output to consecutive files."
    errorecho "Pass at least 4 arguments, that is:"
    errorecho " 1. >>1<< if you want to run in silent mode and >>0<< otherwise"
    errorecho " 2. >>1<< if you do want the sudo session to terminate and >>0<< otherwise"
    errorecho " 3. number of times to execute,"
    errorecho " 4. file name pattern,"
    errorecho " 5. the command to execute along with its parameters."
    errorecho "Please enter your password to the following sudo in order to run the >>perf.sh<< script (if prompted)."
fi
# if [[ $? -ne 0 ]]
if ! sudo -v
then
    errorecho "Sudo fail. Insufficient priviliges to run the perf command. Continuing."
fi

# ${A[@]:1} is tail of A array
# https://stackoverflow.com/questions/1335815/how-to-slice-an-array-in-bash
TIMES_TO_RUN="$3"
FILE_NAME_PATTERN="$4"
COMMAND="${@:5}"

echo 'times to run: ' $TIMES_TO_RUN
echo 'file name pattern: ' $FILE_NAME_PATTERN
echo 'command: ' $COMMAND
echo ' '

# search for the last file which follows the $FILE_NAME_PATTERN$counter pattern
START_COUNT=0
FILE_NAME=$FILE_NAME_PATTERN.$START_COUNT
while [ -e "$FILE_NAME" ]
do
    printf -v FILE_NAME -- '%s' "$FILE_NAME_PATTERN" "." "$(( ++START_COUNT ))"
done

printf 'Will use "%s" as the first filename\n' "$FILE_NAME"


LAST_COUNT=$((START_COUNT + TIMES_TO_RUN ))
COUNT=$START_COUNT
while [ $COUNT -le $LAST_COUNT ]
do
    errorecho $FILE_NAME
    touch $FILE_NAME

    # ./perf.sh prints its output to STDERR and its command's ouput to STDIN
    # Redirect ./perf.sh output to appriopriate file.
    
    sudo ./perf.sh $COMMAND 2> $FILE_NAME

    printf -v FILE_NAME -- '%s' "$FILE_NAME_PATTERN" "." "$(( COUNT++ ))"
done

# # terminate sudo session
# if [[ $2 ]]
# then
#  sudo -k
# fi

# https://unix.stackexchange.com/questions/114239/number-parsing-in-awk/115594
# change locale to LC_ALL=en_US.UTF-8 because of decimal separator-related issues

errorecho "Now I will pass you average data, ma chère"
# LC_ALL=en_US awk --use-lc-numeric '{gsub(/,/,"",$1);a[FNR]+=$1;b[FNR]++;}END{for(i=1;i<=FNR;i++)print i,a[i]/b[i];}' "$FILE_NAME_PATTERN".*
LC_ALL=en_US awk --use-lc-numeric '{gsub(/,/,"",$1);a[FNR]+=$1;b[FNR]++;}END {for(i=1;i<=FNR;i++) printf "%10f\n",a[i]/b[i];}' "$FILE_NAME_PATTERN".* > $FILE_NAME_PATTERN-average
errorecho "Average values saved to $FILE_NAME_PATTERN-average file"
