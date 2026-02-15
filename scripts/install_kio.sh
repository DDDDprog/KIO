#!/usr/bin/env bash
# Axeon Advanced Installer
# GitHub: https://github.com/DDDDprog/Axeon
# SPDX-License-Identifier: Zeo

set -euo pipefail

#########################################
# CONFIGURATION
#########################################
REPO_URL="https://github.com/DDDDprog/Axeon.git"
INSTALL_DIR="/usr/local/axeon"
BUILD_DIR="$HOME/.axeon_build"
ENABLE_JIT="ON"
ENABLE_PARALLEL="ON"
ENABLE_LSP="ON"
ENABLE_FAST_MATH="ON"
ENABLE_LTO="ON"
NUM_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

#########################################
# HELPER FUNCTIONS
#########################################
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

echo_banner() {
    echo -e "\n\033[1;34m=== $1 ===\033[0m\n"
}

#########################################
# DEPENDENCY CHECK
#########################################
echo_banner "Checking system dependencies"

DEPENDENCIES=("git" "cmake" "make" "g++")
for cmd in "${DEPENDENCIES[@]}"; do
    if ! command_exists "$cmd"; then
        echo "Error: $cmd is required. Please install it and rerun this script."
        exit 1
    fi
done

#########################################
# CLONE OR UPDATE REPO
#########################################
echo_banner "Fetching Axeon source code from $REPO_URL"
if [ ! -d "$BUILD_DIR" ]; then
    git clone "$REPO_URL" "$BUILD_DIR"
else
    cd "$BUILD_DIR"
    git pull
fi

#########################################
# BACKUP OLD INSTALLATION
#########################################
if [ -d "$INSTALL_DIR" ]; then
    echo_banner "Backing up old Axeon installation"
    TIMESTAMP=$(date +%Y%m%d_%H%M%S)
    mv "$INSTALL_DIR" "${INSTALL_DIR}_backup_$TIMESTAMP"
fi

#########################################
# BUILD AXEON
#########################################
echo_banner "Configuring Axeon build"
mkdir -p "$BUILD_DIR/build"
cd "$BUILD_DIR/build"

cmake .. \
    -DAXEON_ENABLE_JIT=$ENABLE_JIT \
    -DAXEON_ENABLE_PARALLEL=$ENABLE_PARALLEL \
    -DAXEON_BUILD_LSP=$ENABLE_LSP \
    -DAXEON_ENABLE_FAST_MATH=$ENABLE_FAST_MATH \
    -DAXEON_ENABLE_LTO=$ENABLE_LTO \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"

echo_banner "Building Axeon using $NUM_CORES cores"
cmake --build . -j"$NUM_CORES"

#########################################
# INSTALL AXEON
#########################################
echo_banner "Installing Axeon to $INSTALL_DIR"
cmake --install . --prefix "$INSTALL_DIR"

#########################################
# UPDATE PATH
#########################################
SHELL_RC="$HOME/.bashrc"
if [ -n "$ZSH_VERSION" ]; then
    SHELL_RC="$HOME/.zshrc"
fi

if ! grep -q "$INSTALL_DIR/bin" <<< "$PATH"; then
    echo "export PATH=\"$INSTALL_DIR/bin:\$PATH\"" >> "$SHELL_RC"
    echo "Added Axeon to PATH. Restart your shell or run 'source $SHELL_RC'."
fi

#########################################
# FINAL MESSAGE
#########################################
echo_banner "✅ Axeon installation complete!"
echo "Run an example: axeon $BUILD_DIR/examples/hello.axe"
echo "You can now use Axeon from anywhere using 'axeon'"
