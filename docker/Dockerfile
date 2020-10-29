FROM ubuntu:18.04

LABEL author="Austin Raney"
LABEL author-email="aaraney@crimson.ua.edu"

ARG URL="https://api.github.com/repos/r-barnes/ArcRasterRescue/releases/latest"
ARG FILENAME="ArcRasterRescue.tar.gz"

RUN apt-get update && \
    apt-get install -y\
    cmake libgdal-dev \
    zlib1g-dev g++ \
    wget && \
    mkdir build && \
    # Get url of latest release
    wget -O "${FILENAME}" \
    `wget -qO- "${URL}" | grep -o "https://api.github.com/repos/r-barnes/ArcRasterRescue/tarball/[^\"]*"` && \
    # Extract and build
    tar xf "${FILENAME}" -C build/ --strip-components=1 && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo . &&\
    make && \
    # Soft link to /bin
    ln -s `realpath arc_raster_rescue.exe` /bin/arc_raster_rescue.exe && \
    # Clean up: remove unneeded packages and clear cache
    apt-get remove -y \
    cmake g++ \
    wget &&\
    apt-get clean

WORKDIR /home