ALL=
while IFS=, read -r col1 col2
do
   ALL=$ALL$col1,
done < dtc.csv
echo -n $ALL | sed 's/.$//'

