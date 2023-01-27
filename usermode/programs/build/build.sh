#!/bin/bash
# ./build.sh sourceFolderPath outputPath
arch=i386
compiler=$arch-elf-g++
sourceFolder=$1
outputPath=$2

rm -rf $outputPath

sources=($(find $sourceFolder/** -name "*.cxx") $(find ../libraries/** -name "*.cxx"))
objects=()
for src in "${sources[@]}"; do
	$compiler -c $src -std=c++17 -ffreestanding -fno-exceptions -fno-rtti -O0 -Wall -Wextra -m32 -I ../libraries/ -o $src.o
	objects+=( $src.o )
done

rm -f crt0.o
nasm -felf crt0.s -o crt0.o

objects=$(printf " %s" "${objects[@]}")
$compiler crt0.o $objects -ffreestanding -O0 -nostdlib -nodefaultlibs -T link.ld -m32 -o $outputPath

rm -f ./../libraries/**/*.o