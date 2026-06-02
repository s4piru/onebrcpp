#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

CLASSES_DIR=1brc/target/classes
NUM_ROWS="${1:-1000000000}"

echo "=== Compiling Java sources ==="
mkdir -p "$CLASSES_DIR"
javac -d "$CLASSES_DIR" \
  1brc/src/main/java/dev/morling/onebrc/CreateMeasurements.java \
  1brc/src/main/java/dev/morling/onebrc/CalculateAverage_baseline.java

echo "=== Generating measurements.txt ($NUM_ROWS rows) ==="
java --class-path "$CLASSES_DIR" dev.morling.onebrc.CreateMeasurements "$NUM_ROWS"

echo "=== Running Java reference solution ==="
java --class-path "$CLASSES_DIR" dev.morling.onebrc.CalculateAverage_baseline > expected_output.txt

echo "=== Done ==="
echo "Generated: measurements.txt ($(wc -l < measurements.txt) lines)"
echo "Generated: expected_output.txt ($(wc -c < expected_output.txt) bytes)"
