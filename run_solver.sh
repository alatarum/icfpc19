#!/bin/sh -u

usage()
{
    echo "Usage: $(basename "$0") " \
        "-i <dir_parth> "
}


input_dir=
output_dir=
solver=bin/Debug/icfpc19

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

for file in ${input_dir}/*.desc;
do
    out_filename="$(basename "$file" .desc).sol"
    echo "running for file ${file}"
    $solver -i ${file} -o ${output_dir}/${out_filename}
    if [ $? -ne 0 ]; then
        echo "Failed on file ${file}!"
#        exit
    fi
    echo "Ok"
done
