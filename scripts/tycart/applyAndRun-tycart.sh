#!/bin/bash

target=$1
pathToPlugin=${2:-build/lib}
pluginArgs=${3:-"-typeart -typeart-alloca -typeart-stats -alloca-array-only=false"}
pathToRT=${4:-"$pathToPlugin"/../runtime}
scriptDir=$(dirname "$0")

tmpDir="./"
tmpfile="$tmpDir"/"${target##*/}"
extension="${target##*.}"

rtDir="$( cd "$pathToRT" && pwd )"

echo -e Running on "$target" using plugin: "$plugin"

echo $pathToPlugin
echo $pathToRT

if [ $extension == "c" ]; then
  compiler=clang
else
  compiler=clang++
fi

if [ -e "${tmpDir}/types.yaml" ]; then
    rm "${tmpDir}/types.yaml"
fi

$compiler -S -emit-llvm "$target" -o "$tmpfile".ll -I${scriptDir}/../typelib -I${scriptDir}/../runtime -I${scriptDir}/../runtime/tycart
opt -load ${pathToPlugin}/analysis/meminstfinderpass.so -load ${pathToPlugin}/typeartpass.so -typeart $pluginArgs < "$tmpfile".ll -o "$tmpfile".ll > /dev/null
llc "$tmpfile".ll -o "$tmpfile".s
$compiler "$tmpfile".s -L"$pathToRT" -ltypeart-rt -L"$pathToRT/tycart" -ltycart-rt -o "$tmpfile".o
echo -e Executing with runtime lib
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$rtDir:$rtDir/tycart" "$tmpfile".o
