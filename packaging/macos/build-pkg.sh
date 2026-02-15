#!/usr/bin/env bash
set -euo pipefail

VERSION="${1:-0.0.1}"
BINARY="$2"

STAGING=$(mktemp -d)
mkdir -p "$STAGING/usr/local/bin"
cp "$BINARY" "$STAGING/usr/local/bin/peerchat"
chmod 755 "$STAGING/usr/local/bin/peerchat"

pkgbuild \
    --root "$STAGING" \
    --identifier "com.rapoyrazoglu.peerchat" \
    --version "$VERSION" \
    --install-location "/" \
    "peerchat-${VERSION}-macos.pkg"

rm -rf "$STAGING"
echo "Created peerchat-${VERSION}-macos.pkg"
