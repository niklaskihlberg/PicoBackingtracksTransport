#!/bin/bash
set -e

SRC="ableton_remote/NiklasBTControl"
DST="/Applications/Ableton Live 12 Beta.app/Contents/App-Resources/MIDI Remote Scripts/NiklasBTControl"

# Kontrollera att du har rättigheter
if [ ! -w "$DST" ]; then
  echo "Du har inte skrivbehörighet till $DST. Prova med sudo."
  exit 1
fi

echo "Rensar $DST ..."
rm -rf "$DST"/*

echo "Kopierar från $SRC till $DST ..."
cp -R "$SRC"/* "$DST"/

echo "Klar!"