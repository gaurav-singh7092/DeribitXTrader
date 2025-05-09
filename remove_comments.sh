#!/bin/bash

# Find all source files in src, include, and test directories
find ./src ./include -type f -name "*.cpp" -o -name "*.h" -o -name "*.hpp" > source_files.txt

# Process each file to remove comments
while IFS= read -r file; do
  echo "Processing $file"
  # Remove single-line comments (//...)
  sed -i '' 's/\/\/.*$//' "$file"
  # Remove multi-line comments (/* ... */)
  sed -i '' 's/\/\*.*\*\///g' "$file"
  # Remove empty lines that might have been created
  sed -i '' '/^[[:space:]]*$/d' "$file"
done < source_files.txt

echo "All comments have been removed."
