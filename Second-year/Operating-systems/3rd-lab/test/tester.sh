#!/bin/bash

total_tests=0
passed_tests=0

RunTests() {
    test_files=("$@")
    for test_file in "${test_files[@]}"; do
        if [ ! -f "$test_file" ]; then
            echo "Error: Test file $test_file not found!"
            continue
        fi
        file_basename=$(basename "$test_file")
        echo -e "\nTest $file_basename: is running..."
        ./factory $test_file > MyOutputs/$file_basename 2>&1
        python3 validate_files.py MyOutputs/${file_basename} ExpectedOutputs/${file_basename}
        if [ $? -eq 0 ]; then
            echo "Test: $file_basename: PASSED"
            passed_tests=$((passed_tests + 1))
        else
            echo "Test: $file_basename: FAILED"
        fi
        total_tests=$((total_tests + 1))
    done
}

CompareFiles() {
    files_names=("$@")
    for file_name in "${files_names[@]}"; do
        file_basename=$(basename "$file_name")
        # run a python program to compare the output files.
        python3 validate_files.py MyOutputs/${file_basename} ExpectedOutputs/${file_basename}

        if [ $? -eq 0 ]; then
            echo "Test $file_basename: PASSED"
        else
            echo "Test $file_basename: FAILED"
        fi

    done
}

# Check if the script is run with the correct number of arguments
# Requires 1 argument: the zip file name
CHECK_FILENAME() {
    # Check the format of the zip file.
    pattern="^ssoo_p3_([0-9]+)(_[0-9]+)?(_[0-9]+)?\.zip$"
    if echo "$1" | grep -qE "$pattern"; then
        #echo "Valid format: $zip_filename"
        NIA1=$(echo "$1" | sed -E 's/^ssoo_p3_([0-9]+)(_[0-9]+)?(_[0-9]+)?\.zip$/\1/')
        NIA2=$(echo "$1" | sed -E 's/^ssoo_p3_([0-9]+)(_[0-9]+)?(_[0-9]+)?\.zip$/\2/' | sed 's/^_//')
        NIA3=$(echo "$1" | sed -E 's/^ssoo_p3_([0-9]+)(_[0-9]+)?(_[0-9]+)?\.zip$/\3/' | sed 's/^_//')
        echo "NIA1=$NIA1, NIA2=${NIA2:-None}, NIA3=${NIA3:-None}"
    else
        echo "[ENG] Invalid format: $1, the expected format is ssoo_p3_NIA1_NIA2_NI3.zip"
        echo "[ESP] Formato no válido: $1, el formato esperado es ssoo_p3_NIA1_NIA2_NI3.zip"
        return 1
    fi
    return 0
}

CHECK_AUTHORS_FILE() {
    echo "Reading autores.txt"
    authors_file=autores.txt
    if [ ! -f ${authors_file} ]; then
        echo "Missing $authors_file file or has a bad name"
        echo "Falta el fichero $authors_file"
        return 1
    else
        echo "[ENG] NIA,Surname,First name"
        echo "[ESP] NIA,Apellidos,Nombre"
        cat $authors_file
    fi

    pattern="^[0-9]+,[[:alpha:] ]+,[[:alpha:] ]+$"
    line_number=0
    all_valid=1
    # Eliminar espacios al inicio/final y normalizar
    sed -i 's/^[[:space:]]*//;s/[[:space:]]*$//' "$authors_file"
    while IFS= read -r line; do
        line_number=$((line_number + 1))
        if [[ -z "${line}" ]] || [[ "$line" =~ ^[[:space:]]*$ ]]; then
            echo "[ENG] Skipping empty line $line_number."
            echo "[ESP] Omitiendo la línea vacía $line_number."
            continue
        fi
        if ! echo "$line" | grep -qE "$pattern"; then
            printf "\n"
            echo "[ENG] Error: Line $line_number has an invalid format: $line"
            echo "[ESP] Error: La línea $line_number tiene un formato inválido: $line"
            all_valid=0
        fi
    done <"$authors_file"

    # Check if all lines are valid
    if [ $all_valid = "0" ]; then
        return 1
    fi

    return 0
}

# Check the number of arguments
if [ $# != 1 ]; then
    echo "[ENG] Usage: $0 ssoo_p3_NIA1_NIA2_NIA3.zip, see 'README-ENG.md' for more details."
    echo "[ESP] Uso: $0 ssoo_p3_NIA1_NIA2_NIA3.zip,  véase 'README-ESP.md' para más detalles."
    exit 1
fi

zip_filename="$1"
# Check if the zip exists.
if [ ! -f $zip_filename ]; then
    echo "[ENG] $zip_filename does not exist."
    echo "[ESP] $zip_filename no existe."
    exit 1
fi

if ! CHECK_FILENAME $zip_filename; then # bad formated.
    exit 1
fi

# Unzip the student code
out_dir=$(basename $zip_filename .zip)

if [ -d $out_dir ]; then
    rm -rf $out_dir
fi

unzip -j -o $1 -d $out_dir

# Move to the directory where the student code is
cd $out_dir

# Compile the student code
make

# Check if the compilation was successful
if [ $? -ne 0 ]; then
    echo "[ENG] Compilation failed."
    echo "[ESP] La compilación ha fallado."
    exit 1
fi

# Check authors file
if ! CHECK_AUTHORS_FILE; then # bad formated.
    exit 1
fi

# Copy files to the student directory
cp -r ../ExpectedOutputs/ .
cp -r ../InputFiles/ .
cp ../validate_files.py .


# Create the output directory
mkdir ./MyOutputs

# Get a list of all the .err files in the InputFiles directory
err_files=($(ls InputFiles/*.err | sort))
echo ${err_files[@]}

# Get a list of all the .txt files in the InputFiles directory
valid_files=($(ls InputFiles/*.txt | sort))
echo ${valid_files[@]}

RunTests "${err_files[@]}"
# Run valid tests 10 times.
for i in {1..10}; do 
    echo "+++ Iteration / Iteración: $i"
    RunTests "${valid_files[@]}" 
    echo "-------------------------------"
done

echo " ########### Summary / Resumen ########## "
echo "Passed tests / Pruebas superadas: ${passed_tests}/${total_tests}"

# CompareFiles "${err_files[@]}"
# CompareFiles "${valid_files[@]}" 




