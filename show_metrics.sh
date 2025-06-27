#!/usr/bin/env bash

rm -Rf cloc_match_only
mkdir cloc_match_only


cp *.cpp cloc_match_only
cp *.h cloc_match_only
cp *.pro cloc_match_only
cp *.qm cloc_match_only
cp *.sh cloc_match_only

echo "Show CODE METRICS ..."
cloc cloc_match_only
rm -Rf cloc_match_only 
