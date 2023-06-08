#!/bin/bash
cd lua_jit
make CFLAGS='-fPIC'
sudo make install