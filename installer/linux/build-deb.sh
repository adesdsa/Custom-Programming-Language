#!/bin/bash
# ============================================================
# picoC Debian Package Builder
# ============================================================
# Linux equivalent of picoc-setup.iss (Inno Setup on Windows).
#
# Build with:  ./build-deb.sh
# Or with GUI: ./build-deb.sh --with-gui
# Cross-compile: ./build-deb.sh --arch=i386
#
# Prerequisites:
#   sudo apt install build-essential cmake nasm dpkg-dev
#   (for GUI:   sudo apt install qt6-base-dev libqt6svg6-dev)
#   (for i386:  sudo apt install gcc-multilib g++-multilib)
# ============================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

APP_NAME="picoc"
APP_VERSION="1.0.0"
ARCH="$(dpkg --print-architecture)"
BUILD_GUI=false

# ---- Parse arguments ----
for arg in "$@"; do
    case "$arg" in
        --with-gui)  BUILD_GUI=true ;;
        --version=*) APP_VERSION="${arg#*=}" ;;
        --arch=*)    ARCH="${arg#*=}" ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --with-gui       Build and include the Qt GUI (requires Qt 6)"
            echo "  --version=X.Y.Z  Set the package version (default: $APP_VERSION)"
            echo "  --arch=ARCH      Target architecture: amd64 or i386 (default: host)"
            echo "  -h, --help       Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $arg"
            exit 1
            ;;
    esac
done

PACKAGE_NAME="${APP_NAME}_${APP_VERSION}_${ARCH}"
OUTPUT_DIR="$SCRIPT_DIR/output"
STAGING_DIR="$OUTPUT_DIR/$PACKAGE_NAME"

echo "============================================="
echo "  picoC Debian Package Builder v${APP_VERSION}"
echo "  Architecture: ${ARCH}"
echo "============================================="
echo ""

# ---- Check prerequisites ----
echo "[1/6] Checking prerequisites..."
for cmd in cmake dpkg-deb strip; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo "ERROR: '$cmd' is not installed."
        echo "  Install with: sudo apt install cmake dpkg-dev binutils"
        exit 1
    fi
done

if ! command -v nasm >/dev/null 2>&1; then
    echo "WARNING: 'nasm' is not installed. It will be a runtime dependency."
    echo "  Install with: sudo apt install nasm"
fi

if [ "$ARCH" = "i386" ]; then
    if ! dpkg -s gcc-multilib >/dev/null 2>&1 || ! dpkg -s g++-multilib >/dev/null 2>&1; then
        echo "ERROR: 32-bit cross-compilation packages not found."
        echo "  Install with: sudo apt install gcc-multilib g++-multilib"
        exit 1
    fi
fi

if [ "$BUILD_GUI" = true ]; then
    if ! dpkg -s qt6-base-dev >/dev/null 2>&1; then
        echo "ERROR: Qt 6 development packages not found."
        echo "  Install with: sudo apt install qt6-base-dev libqt6svg6-dev"
        exit 1
    fi
fi

echo "  All prerequisites OK."
echo ""

# ---- Build from source ----
echo "[2/6] Building picoC from source..."
BUILD_DIR="$PROJECT_ROOT/build-deb-${ARCH}"
CMAKE_ARGS=(-DCMAKE_BUILD_TYPE=Release)

if [ "$ARCH" = "i386" ]; then
    CMAKE_ARGS+=(-DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32)
fi

if [ "$BUILD_GUI" = true ]; then
    CMAKE_ARGS+=(-DBUILD_GUI=ON)
fi

# Reuse cached FetchContent dependencies from a prior build if available
EXISTING_DEPS=""
for candidate in "$PROJECT_ROOT/build" "$PROJECT_ROOT/build-deb-amd64" "$PROJECT_ROOT/build-deb-i386" "$PROJECT_ROOT/build-deb"; do
    if [ -d "$candidate/_deps" ]; then
        EXISTING_DEPS="$candidate/_deps"
        break
    fi
done

if [ -n "$EXISTING_DEPS" ] && [ ! -d "$BUILD_DIR/_deps" ]; then
    echo "  Reusing cached dependencies from $EXISTING_DEPS"
    mkdir -p "$BUILD_DIR/_deps"
    cp -r "$EXISTING_DEPS"/. "$BUILD_DIR/_deps/"
    CMAKE_ARGS+=(-DFETCHCONTENT_FULLY_DISCONNECTED=ON)
fi

if [ -d "$BUILD_DIR/_deps" ]; then
    CMAKE_ARGS+=(-DFETCHCONTENT_FULLY_DISCONNECTED=ON)
fi

cmake -B "$BUILD_DIR" -S "$PROJECT_ROOT" "${CMAKE_ARGS[@]}"
cmake --build "$BUILD_DIR" --config Release -j"$(nproc)"

echo "  Build complete."
echo ""

# ---- Prepare staging directory ----
echo "[3/6] Preparing package structure..."
rm -rf "$STAGING_DIR"

mkdir -p "$STAGING_DIR/DEBIAN"
mkdir -p "$STAGING_DIR/usr/bin"

if [ "$BUILD_GUI" = true ]; then
    mkdir -p "$STAGING_DIR/usr/share/icons/hicolor/128x128/apps"
    mkdir -p "$STAGING_DIR/usr/share/applications"
fi

echo "  Directory structure created."
echo ""

# ---- Copy files ----
echo "[4/6] Copying files..."

# CLI executable
CLI_BIN="$BUILD_DIR/Custom-Programming-Language"
if [ ! -f "$CLI_BIN" ]; then
    echo "ERROR: CLI binary not found at $CLI_BIN"
    exit 1
fi
cp "$CLI_BIN" "$STAGING_DIR/usr/bin/picoc"
chmod 755 "$STAGING_DIR/usr/bin/picoc"
strip --strip-unneeded "$STAGING_DIR/usr/bin/picoc"
echo "  Installed: picoc -> /usr/bin/picoc"

# GUI executable (optional)
if [ "$BUILD_GUI" = true ]; then
    GUI_BIN="$BUILD_DIR/picoc-gui"
    if [ ! -f "$GUI_BIN" ]; then
        echo "ERROR: GUI binary not found at $GUI_BIN"
        exit 1
    fi
    cp "$GUI_BIN" "$STAGING_DIR/usr/bin/picoc-gui"
    chmod 755 "$STAGING_DIR/usr/bin/picoc-gui"
    strip --strip-unneeded "$STAGING_DIR/usr/bin/picoc-gui"
    echo "  Installed: picoc-gui -> /usr/bin/picoc-gui"

    cp "$SCRIPT_DIR/picoc-gui.desktop" "$STAGING_DIR/usr/share/applications/"
    chmod 644 "$STAGING_DIR/usr/share/applications/picoc-gui.desktop"
    echo "  Installed: picoc-gui.desktop -> /usr/share/applications/"

    ICON_SRC="$PROJECT_ROOT/resources/icons/logo.png"
    if [ -f "$ICON_SRC" ]; then
        cp "$ICON_SRC" \
           "$STAGING_DIR/usr/share/icons/hicolor/128x128/apps/picoc.png"
        chmod 644 "$STAGING_DIR/usr/share/icons/hicolor/128x128/apps/picoc.png"
        echo "  Installed: picoc.png -> /usr/share/icons/"
    else
        echo "  WARNING: Icon not found at $ICON_SRC — skipping icon installation."
        echo "           The GUI will work but won't have an application icon."
    fi
fi

echo ""

# ---- Generate DEBIAN/control ----
echo "[5/6] Generating package metadata..."

if [ "$BUILD_GUI" = true ]; then
    DEPENDS="nasm, binutils, libc6, libqt6widgets6 (>= 6.2), libqt6svg6 (>= 6.2), qt6-qpa-plugins"
    PKG_DESCRIPTION="picoC Compiler - a custom programming language toolchain
 picoC is a compiler and IDE for the picoC programming language.
 This package provides the command-line compiler (picoc) and
 the graphical IDE (picoc-gui) built with Qt 6."
else
    DEPENDS="nasm, binutils, libc6"
    PKG_DESCRIPTION="picoC Compiler - a custom programming language toolchain
 picoC is a compiler and IDE for the picoC programming language.
 This package provides the command-line compiler (picoc)."
fi

INSTALLED_SIZE=$(du -sk "$STAGING_DIR" | cut -f1)

cat > "$STAGING_DIR/DEBIAN/control" <<CTRL
Package: ${APP_NAME}
Version: ${APP_VERSION}
Section: devel
Priority: optional
Architecture: ${ARCH}
Depends: ${DEPENDS}
Installed-Size: ${INSTALLED_SIZE}
Maintainer: picoC Team <picoc@users.noreply.github.com>
Homepage: https://github.com/picoC
Description: ${PKG_DESCRIPTION}
CTRL

# Generate postinst dynamically based on what was actually packaged
cat > "$STAGING_DIR/DEBIAN/postinst" <<'POSTINST_HEAD'
#!/bin/sh
set -e

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database -q /usr/share/applications/ 2>/dev/null || true
fi

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -q /usr/share/icons/hicolor/ 2>/dev/null || true
fi

echo ""
echo "========================================="
echo "  picoC has been installed successfully!"
echo "========================================="
echo ""
echo "  CLI:  picoc <source-file>"
POSTINST_HEAD

if [ "$BUILD_GUI" = true ]; then
    echo 'echo "  GUI:  picoc-gui"' >> "$STAGING_DIR/DEBIAN/postinst"
fi

echo 'echo ""' >> "$STAGING_DIR/DEBIAN/postinst"

cp "$SCRIPT_DIR/postrm" "$STAGING_DIR/DEBIAN/postrm"
chmod 755 "$STAGING_DIR/DEBIAN/postinst"
chmod 755 "$STAGING_DIR/DEBIAN/postrm"

echo "  Package metadata generated."
echo ""

# ---- Build the .deb ----
echo "[6/6] Building .deb package..."
dpkg-deb --build --root-owner-group "$STAGING_DIR"

DEB_FILE="$OUTPUT_DIR/${PACKAGE_NAME}.deb"
DEB_SIZE=$(du -h "$DEB_FILE" | cut -f1)

echo ""
echo "============================================="
echo "  SUCCESS! Package built:"
echo "  $DEB_FILE ($DEB_SIZE)"
echo "============================================="
echo ""
echo "  Install:    sudo apt install ./${PACKAGE_NAME}.deb"
echo "  Remove:     sudo apt remove ${APP_NAME}"
echo ""
