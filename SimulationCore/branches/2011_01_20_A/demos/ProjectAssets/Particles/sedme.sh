#!/bin/bash
#chmod u+w "$1"
sed 's/\.\.\///g' "$1" > "${1}.out"
#cat "${1}.out"
mv "${1}.out" "$1"
#chmod u-w "$1"
