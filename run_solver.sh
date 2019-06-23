#!/bin/bash -u

usage()
{
    echo "Usage: $(basename "$0") " \
        "-i <dir_parth>     input files directory"
        "-o <dir_parth>     output files directory"
        "-t <n>             process only each N-th file"
        "-n <n>             start from N-th file"
}


input_dir=
output_dir=
solver=./icfpc19

threads=1
start_num=1

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
    -n)
        shift
        if [ ! $# -gt 0 ]; then
            echo "$(basename "$0"): syntax error"
            usage
            exit 1
        fi
        start_num=$1
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

declare -a prob_files=(${input_dir}/*.desc)

# get length of an array
arraylength=${#prob_files[@]}

# use for loop to read all values and indexes
for (( i=start_num; i<${arraylength}+1; i=i+threads));
do
    in_file=${prob_files[$i-1]}
    out_filename="$(basename "${in_file}" .desc).sol"
    echo "running for file ${in_file} (" $i "/" ${arraylength} ")"
    $solver -i ${in_file} -o ${output_dir}/${out_filename} -n 50000
    if [ $? -ne 0 ]; then
        echo "Runner $start_num: Failed on file ${in_file}!"
    else
        echo "Runner $start_num: Ok for file ${in_file}"
    fi
done

echo "Runner $start_num: Done"
