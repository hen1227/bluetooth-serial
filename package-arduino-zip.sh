#!/usr/bin/env bash
set -euo pipefail

LIB_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if ! command -v zip >/dev/null 2>&1; then
  echo "error: zip command not found" >&2
  exit 1
fi

LIB_DISPLAY_NAME="$(awk -F= '/^name=/{ gsub(/\r/, "", $2); print $2; exit }' "$LIB_DIR/library.properties")"
VERSION="$(awk -F= '/^version=/{ gsub(/\r/, "", $2); print $2; exit }' "$LIB_DIR/library.properties")"

LIB_DISPLAY_NAME="${LIB_DISPLAY_NAME:-Bluetooth Serial Connect}"
LIB_FOLDER_NAME="$(printf '%s' "$LIB_DISPLAY_NAME" | tr -cd '[:alnum:]_.-')"
LIB_FOLDER_NAME="${LIB_FOLDER_NAME:-BluetoothSerialConnect}"
VERSION="${VERSION:-dev}"

OUT_DIR="${1:-$LIB_DIR/dist}"
mkdir -p "$OUT_DIR"
OUT_DIR="$(cd "$OUT_DIR" && pwd)"
ZIP_PATH="$OUT_DIR/$LIB_FOLDER_NAME-$VERSION.zip"

STAGE_DIR="$(mktemp -d "${TMPDIR:-/tmp}/${LIB_FOLDER_NAME}.XXXXXX")"
cleanup() {
  rm -rf "$STAGE_DIR"
}
trap cleanup EXIT

PACKAGE_DIR="$STAGE_DIR/$LIB_FOLDER_NAME"
mkdir -p "$PACKAGE_DIR"

copy_if_present() {
  local relative_path="$1"
  local source_path="$LIB_DIR/$relative_path"
  local destination_path="$PACKAGE_DIR/$relative_path"

  if [[ -e "$source_path" ]]; then
    mkdir -p "$(dirname "$destination_path")"
    cp -R "$source_path" "$destination_path"
  fi
}

for path in \
  library.properties \
  keywords.txt \
  README.md \
  LICENSE \
  LICENSE.md \
  src \
  examples
do
  copy_if_present "$path"
done

find "$PACKAGE_DIR" \( \
  -name ".git" -o \
  -name ".github" -o \
  -name ".DS_Store" -o \
  -name "__MACOSX" -o \
  -name "._*" \
\) -exec rm -rf {} +

rm -f "$ZIP_PATH"
(
  cd "$STAGE_DIR"
  zip -qr "$ZIP_PATH" "$LIB_FOLDER_NAME"
)

echo "Created $ZIP_PATH"
