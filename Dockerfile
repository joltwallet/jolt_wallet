# Image used to build firmware and firmware-related artifacts

FROM ubuntu:19.04

WORKDIR /jolt_wallet

RUN ln -s /usr/bin/python3 /usr/bin/python

RUN apt update && apt install -y \
    lsb-release \
    make \
    && apt clean

######################################################
# Copy over the minimum files for a complete install #
######################################################
COPY esp-idf/requirements.txt esp-idf/requirements.txt
COPY esp-idf/tools/ esp-idf/tools/
COPY Makefile .
COPY make/ make/
COPY requirements.txt .

# Install all requirements
RUN make install

# Install Firebase CLI
RUN curl -sL https://firebase.tools | bash

# TODO on run (with mapped in git repo):
# 1. `make install` - make sure everything is up to date
# 2. `source export.sh` - make sure all paths are up to date
# 3. `make compress` - builds `jolt.bin` and compresses `jolt.bin.gz`

# TODO for Firebase
# Upload jolt.bin and jolt.bin.gz to Firebase Cloud Storage and store the return URLs
# Craft DB object entry for Firestore DB with:
# - SemVar
# - Uncompressed Unsigned URL
# - Compressed Unsigned URL