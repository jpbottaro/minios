#!/bin/bash

sudo mount -o loop fsimage tmp && \
sudo cp ../apps/cash tmp/bin/ && \
sudo cp ../apps/lineecho tmp/bin/ && \
sudo cp ../apps/echo tmp/bin/ && \
sudo cp ../apps/argc tmp/bin/ && \
sudo cp ../apps/ls tmp/bin/ && \
sudo cp ../apps/cat tmp/bin/ && \
sudo cp ../apps/cp tmp/bin/ && \
sudo cp ../apps/rm tmp/bin/ && \
sudo umount tmp

