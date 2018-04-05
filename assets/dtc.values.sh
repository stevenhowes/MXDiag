ALL=
while IFS=, read -r col1 col2
do
	echo \"$col2\",
done < dtc.csv

