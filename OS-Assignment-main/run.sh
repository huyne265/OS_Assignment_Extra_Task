#!/bin/bash

EXECUTABLE="./os"
CONFIG_FILE="$1"

if [ -z "$CONFIG_FILE" ]; then
  echo "Usage: $0 [path to configure file]"
  exit 1
fi

# Compile program
if [ ! -f "$EXECUTABLE" ]; then
  echo "Compiling program..."
  make all
  if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
  fi
fi

# Run Program
$EXECUTABLE "$CONFIG_FILE"