#!/bin/bash -u

usage()
{
    echo "Usage: $(basename "$0") " \
        "-i <dir_parth>     input files directory"
        "-o <dir_parth>     output files directory"
        "-t <n>             process only each N-th file"
}


input_dir=
output_dir=
runner=./run_solver.sh

threads=1

while [ $# -gt 0 ];
do
    case "$1" in
    -i)
        shift
        if [ ! $# -gt 0 ]; then
            echo "$(basename "$0"): syntax error"
            usage
            exit 1
        fi
        input_dir=$1
    ;;
    -o)
        shift
        if [ ! $# -gt 0 ]; then
            echo "$(basename "$0"): syntax error"
            usage
            exit 1
        fi
        output_dir=$1
    ;;
    -t)
        shift
        if [ ! $# -gt 0 ]; then
            echo "$(basename "$0"): syntax error"
            usage
            exit 1
        fi
        threads=$1
    ;;
    -h)
        echo "$(basename "$0"):"
        usage
        exit 1
    ;;
    *)
        echo "$(basename "$0"): unrecognized argument: $1"
        usage
        exit 1
    ;;
    esac
    shift
done

if [ -z "$input_dir" ]; then
        echo "Input directory not specified!"
        exit
fi
if [ -z "$output_dir" ]; then
        echo "Output directory not specified!"
        exit
fi
if [ ! -d "$output_dir" ]; then
        echo "Output directory not exists!"
        exit
fi

for (( i=1; i<$threads+1; i++));
do
    echo "Running pool ${i}"
    ${runner}  -i ${input_dir} -o ${output_dir} -t $threads -n $i &
done

