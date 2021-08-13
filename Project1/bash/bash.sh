#!/bin/bash


# Output file name
output="../data"

if [ "$#" -ne 4 ]; 
then
    echo "ERROR: Wrong number of parameters."
    exit -1
fi

if [ ! -f "$1" ];
then
    echo "ERROR: $1 does not exist."
    exit -1
fi

if [ ! -f "$2" ];
then
    echo "ERROR: $2 does not exist."
    exit -1
fi

if [ -f "$output" ];
then
    rm $output
fi

if [[ "$4" -eq 0 ]];
then
	if [[ "$3" -gt 9999 ]];
	then
		echo "ERROR: $3 entries do not support no duplicates mode."
		exit 1
	fi
	ids=($(shuf -i 0-9999 -n "$3"))
fi

counter=1
# Make viruses array
viruses=( $( cat "$1" ) )
# Make countries array
countries=( $( cat "$2" ) )

while [[ "$counter" -lt "$3" ]]
do
	# Start generating entries
	# Initialize available letters for use
	upperChars=ABCDEFGHIJKLMNOPQRSTUVWXYZ
	lowerChars=abcdefghijklmnopqrstuvwxyz

	if [ "$4" -eq 1 ];
	then

		id=$(($RANDOM % 9999 + 1))
		# Generate first name
		fn=$(
		# Generate the minimum of threee characters -- first one capital
		echo -n "${upperChars:RANDOM % ${#upperChars}:1}"
		echo -n "${lowerChars:RANDOM % ${#lowerChars}:1}"
		echo -n "${lowerChars:RANDOM % ${#lowerChars}:1}"
		# Generate rest of characters - max of 9 - for a maximum total of 12
		echo -n "${lowerChars:RANDOM % ${#lowerChars}:9}"
		)
		# Generate last name
		ln=$(
		# Generate the minimum of threee characters -- first one capital
		echo -n "${upperChars:RANDOM % ${#upperChars}:1}"
		echo -n "${lowerChars:RANDOM % ${#lowerChars}:1}"
		echo -n "${lowerChars:RANDOM % ${#lowerChars}:1}"
		# Generate rest of characters - max of 9 - for a maximum total of 12
		echo -n "${lowerChars:RANDOM % ${#lowerChars}:9}"
		)
		# Get random country
		country=${countries[$RANDOM % ${#countries[*]}]}
		# Generate random age
		age=$(($RANDOM % 120 + 1))
		# Get random country
		virus=${viruses[$RANDOM % ${#viruses[*]}]}
		# Generate date
		date=$(
		# Generate the minimum of threee characters -- first one capital
		echo -n "$(($RANDOM % 30 + 1))"
		echo -n "-"
		echo -n "$(($RANDOM % 12 + 1))"
		echo -n "-"
		echo -n "$(($RANDOM % 9999 + 1))"
		)
		# Flip a coin to decide if we give a yes or no for vaccinated on this entry
		yesno=$(($RANDOM % 2))

		if [[ "$yesno" -eq 1 ]];
		then 
			echo "$id $fn $ln $country $age $virus YES $date" >> "$output"
		else
			nodate=$(($RANDOM % 3))
			if [[ "$nodate" -eq 2 ]];	# Low chance for no with date
			then
				echo "$id $fn $ln $country $age $virus NO $date" >> "$output"
			else
				echo "$id $fn $ln $country $age $virus NO" >> "$output"
			fi
		fi

		((counter+=1))

		while [ $(($RANDOM % 3)) -ne 2 ]
		do

			if [ $counter -gt $3 ];
			then
				exit 0
			fi

			# Generate first name
			fn=$(
			# Generate the minimum of threee characters -- first one capital
			echo -n "${upperChars:RANDOM % ${#upperChars}:1}"
			echo -n "${lowerChars:RANDOM % ${#lowerChars}:1}"
			echo -n "${lowerChars:RANDOM % ${#lowerChars}:1}"
			# Generate rest of characters - max of 9 - for a maximum total of 12
			echo -n "${lowerChars:RANDOM % ${#lowerChars}:9}"
			)
			# Generate last name
			ln=$(
			# Generate the minimum of threee characters -- first one capital
			echo -n "${upperChars:RANDOM % ${#upperChars}:1}"
			echo -n "${lowerChars:RANDOM % ${#lowerChars}:1}"
			echo -n "${lowerChars:RANDOM % ${#lowerChars}:1}"
			# Generate rest of characters - max of 9 - for a maximum total of 12
			echo -n "${lowerChars:RANDOM % ${#lowerChars}:9}"
			)
			# Get random country
			country=${countries[$RANDOM % ${#countries[*]}]}
			# Generate random age
			age=$(($RANDOM % 120 + 1))
			# Get random country
			virus=${viruses[$RANDOM % ${#viruses[*]}]}
			# Generate date
			date=$(
			# Generate the minimum of threee characters -- first one capital
			echo -n "$(($RANDOM % 31))"
			echo -n "-"
			echo -n "$(($RANDOM % 13))"
			echo -n "-"
			echo -n "$(($RANDOM % 10000))"
			)
			# Flip a coin to decide if we give a yes or no for vaccinated on this entry
			yesno=$(($RANDOM % 2))

			if [[ "$yesno" -eq 1 ]];
			then 
				echo "$id $fn $ln $country $age $virus YES $date" >> "$output"
			else
				nodate=$(($RANDOM % 3))
				if [[ "$nodate" -eq 2 ]];	# Low chance for no with date
				then
					echo "$id $fn $ln $country $age $virus NO $date" >> "$output"
				else
					echo "$id $fn $ln $country $age $virus NO" >> "$output"
				fi
			fi

			((counter+=1))

		done

	else

		id=${ids[$counter]}
		# Generate first name
		fn=$(
		# Generate the minimum of threee characters -- first one capital
		echo -n "${upperChars:RANDOM % ${#upperChars}:1}"
		echo -n "${lowerChars:RANDOM % ${#lowerChars}:1}"
		echo -n "${lowerChars:RANDOM % ${#lowerChars}:1}"
		# Generate rest of characters - max of 9 - for a maximum total of 12
		echo -n "${lowerChars:RANDOM % ${#lowerChars}:9}"
		)
		# Generate last name
		ln=$(
		# Generate the minimum of threee characters -- first one capital
		echo -n "${upperChars:RANDOM % ${#upperChars}:1}"
		echo -n "${lowerChars:RANDOM % ${#lowerChars}:1}"
		echo -n "${lowerChars:RANDOM % ${#lowerChars}:1}"
		# Generate rest of characters - max of 9 - for a maximum total of 12
		echo -n "${lowerChars:RANDOM % ${#lowerChars}:9}"
		)
		# Get random country
		country=${countries[$RANDOM % ${#countries[*]}]}
		# Generate random age
		age=$(($RANDOM % 120 + 1))
		# Get random country
		virus=${viruses[$RANDOM % ${#viruses[*]}]}
		# Generate date
		date=$(
		# Generate the minimum of threee characters -- first one capital
		echo -n "$(($RANDOM % 31))"
		echo -n "-"
		echo -n "$(($RANDOM % 13))"
		echo -n "-"
		echo -n "$(($RANDOM % 10000))"
		)
		# Flip a coin to decide if we give a yes or no for vaccinated on this entry
		yesno=$(($RANDOM % 2))

		if [[ "$yesno" -eq 1 ]];
		then 
			echo "$id $fn $ln $country $age $virus YES $date" >> "$output"
		else
			nodate=$(($RANDOM % 3))
			if [[ "$nodate" -eq 2 ]];	# Low chance for no with date
			then
				echo "$id $fn $ln $country $age $virus NO $date" >> "$output"
			else
				echo "$id $fn $ln $country $age $virus NO" >> "$output"
			fi
		fi

		((counter+=1))

	fi

done

exit 0