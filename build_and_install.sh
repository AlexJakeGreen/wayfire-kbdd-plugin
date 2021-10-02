#!/usr/bin/env bash

meson build \
    &&ninja -C build \
    && sudo ninja -C build install
