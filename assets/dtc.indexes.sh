ALL=
while IFS=, read -r col1 col2
do
   ALL=$ALL`expr $col1 - 0`,
done < dtc.csv
echo -n $ALL | sed 's/.$//'

