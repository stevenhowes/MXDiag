#!/bin/bash

# ---------- svg.h ----------
gzip --best -k originals/go.svg
./bin2hex --i originals/go.svg.gz --o go.svg.hex > /dev/null
rm -f originals/go.svg.gz

gzip --best -k originals/mz.svg
./bin2hex --i originals/mz.svg.gz --o mz.svg.hex >/dev/null
rm -f originals/mz.svg.gz

cat svg.h.top go.svg.hex svg.h.mid mz.svg.hex svg.h.tail > ../svg.h
rm -f go.svg.hex
rm -f mz.svg.hex



# ---------- manifest.h ----------
gzip --best -k originals/manifest.json
./bin2hex --i originals/manifest.json.gz --o manifest.json.hex > /dev/null
rm -f originals/manifest.json.gz

cat manifest.h.top manifest.json.hex manifest.h.tail > ../manifest.h
rm -f manifest.json.hex



# ---------- icon.h ----------
gzip --best -k originals/icon.png
./bin2hex --i originals/icon.png.gz --o icon.png.hex > /dev/null
rm -f originals/icon.png.gz

cat icon.h.top icon.png.hex icon.h.tail > ../icon.h
rm -f icon.png.hex



# ---------- index.h ----------
gzip --best -k originals/index.html
./bin2hex --i originals/index.html.gz --o index.html.hex > /dev/null
rm -f originals/index.html.gz

cat index.h.top index.html.hex index.h.tail > ../index.h
rm -f index.html.hex



# ---------- jquery.h ----------
gzip --best -k originals/jq.js
./bin2hex --i originals/jq.js.gz --o jq.js.hex > /dev/null
rm -f originals/jq.js.gz

cat jquery.h.top jq.js.hex jquery.h.tail > ../jquery.h
rm -f jq.js.hex



# ---------- javascript.h ----------
gzip --best -k originals/mx.js
./bin2hex --i originals/mx.js.gz --o mx.js.hex > /dev/null
rm -f originals/mx.js.gz

cat javascript.h.top mx.js.hex javascript.h.tail > ../javascript.h
rm -f mx.js.hex



# ---------- dtc.h ----------
echo "// Auto-generated - edit asset file dtc.csv" > ../dtc.h
echo const int dtc_index [] PROGMEM = {`./dtc.indexes.sh`}\; >> ../dtc.h
echo const char *dtc_data[`./dtc.values.sh | wc -l`]  = { >> ../dtc.h
./dtc.values.sh  >> ../dtc.h
echo }\; >> ../dtc.h
