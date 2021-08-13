#!/bin/bash

# Check if input file exists
if [[ ! -f "$1" ]];
then
    echo "File $1 does not exist."
    exit -1
fi
# Check if output dir exists
if [[ -d "$2" ]];
then
    echo "Directory $2 exists on your filesystem."
    exit -1
fi
# Check if numFilesPerDirectory is positive
if [[ "$3" -lt 1 ]];
then
    echo "numFilesPerDirectory cannot be less than 1."
    exit -1
fi
# Make output dir
mkdir "$2"
# Read input line by line
while IFS= read -r line
do
	# Display line and grab 4th word -- Grab country from each line 
  	country=$(echo $line | awk '{print $4}')
  	# Check if a directory exists for this country - if not, create it
  	if [[ ! -d "$2/$country" ]];
	then
		# Here we have a country we have not seen before, so we create its structure
	    mkdir "$2/$country"
	    # Create numFilesPerDirectory number of txt files 
	    for ((i=1; i<="$3"; i++))
		do
		   touch "$2/$country/$country-$i.txt"
		done
	fi
	# Get number of lines for first and last file
	n=$(wc -l "$2/$country/$country-1.txt" | awk '{ print $1 }')
	n_last=$(wc -l "$2/$country/$country-$3.txt" | awk '{ print $1 }')
	# If lines of first and last file are equal, it is time to write on first file
	if [[ "$n" -eq "$n_last" ]];
	then
		echo "$line" >> "$2/$country/$country-1.txt"
		continue	# Skip rest code for this line
	fi
	# Find correct file to append line -- compare each file with its previous, if it has less lines then append here
    for ((i=2; i<="$3"; i++))
	do
		t=$(wc -l "$2/$country/$country-$i.txt" | awk '{ print $1 }')

		if [[ "$t" -lt "$n" ]]; 
		then
			echo "$line" >> "$2/$country/$country-$i.txt"
			break 	# Skip checking rest pairs
		fi

		n=$t
	done

done < "$1"