#!/bin/bash
# Copyright 2019 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This assumes it is being run inside a docker container with the following
# mounts:
#  /SRC: Swarming start dir
#  /OUT: output directory for gn and ninja

set -e
set -x

export PATH="/SRC/cipd_bin_packages/cpython3/bin:/SRC/recipe_bundle/depot_tools:${PATH}"

env

cd /SRC/skia
./bin/fetch-ninja
./bin/fetch-gn
./bin/gn gen /OUT "--args=$1"
./third_party/ninja/ninja -C /OUT

chmod -R 0777 /OUT/*
