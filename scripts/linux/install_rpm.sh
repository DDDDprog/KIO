#!/bin/bash
# Install KIO on RPM-based systems (Fedora, CentOS, RHEL)
sudo dnf check-update
sudo dnf install -y cmake gcc-c++ make
cd ..
cmake -B build
cmake --build build -j$(nproc)
sudo cmake --install build
echo "KIO installed on RPM-based system!"
