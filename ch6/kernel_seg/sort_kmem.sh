# Reverse sort kernel mem map by descending address
awk -F'=' '{val="0x" $2; sub("^0x0x","0x",val); print strtonum(val),$0 ;}' kmem.txt |sort -nr |sed 's/^[^ ]* //'
