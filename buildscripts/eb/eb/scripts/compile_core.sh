#!/usr/bin/env bash

bypass_compile=false
targets=core

# TODO setup python virtualenv for Server
pip install -r etc/pip/dev-requirements.txt

# TODO: change CWD to scripts/
source ./buildscripts/eb/eb/scripts/_scons_compile.sh