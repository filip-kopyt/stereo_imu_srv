# User-space server application for "stereo_imu" shield

Shares data obtailed by the kernel driver and exposes them via TCP link.

    mkdir build
    cd build
    cmake ..
    make
    ./srv