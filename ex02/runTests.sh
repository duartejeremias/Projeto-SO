#! /bin/bash

if [ ! $# -eq 3 ]
then
    echo "Error: Incorrect number of arguments given."
    exit 1
fi

inputdir="${1}" 
outputdir="${2}"
maxthreads="${3}"

# argument verification
if [ ! -d $inputdir ]
then
    echo "Error: Input directory does not exist."
    exit 1
elif [ ! -d $outputdir ]
then
    echo "Error: Output directory does not exist."
    exit 1
elif [ $maxthreads -lt 1 ]
then
    echo "Error: Invalid number of threads."
    exit 1
fi

echo "-------------------------------------------------------"

for testFile in $inputdir/*.txt; do
    
    for ((x = 1; x <= maxthreads; x++)) ; do
        echo
        fileName=$(basename $testFile .txt)
        echo -e "InputFile=$(basename $testFile) NumThreads=$x"
        ./tecnicofs $testFile $outputdir/$fileName-$x.txt $x 2> /dev/null | grep "completed"
        echo
    done
    echo "-------------------------------------------------------"
done