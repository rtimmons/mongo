#!/usr/bin/env bash

bypass_compile=false
targets=core

# TODO setup python virtualenv for Server
pip install -r etc/pip/dev-requirements.txt

# TODO: change CWD to scripts/
source ./buildscripts/eb/eb/scripts/_scons_compile.sh

# TODO
# mv mongo ./build/Artifacts/mongo

echo "jsCore-1374" >> ./build/Tasks/001
echo "jsCore-9347324" >> ./build/Tasks/001
# generates a task that calls 'eb jsCore-1374'
