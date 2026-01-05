#!/bin/bash

cp -r /work /build
cd /build
kibot -c kibot.yml -e ASPECT2.kicad_sch -d output_docker/
rm -rf /work/output_docker
cp -r output_docker /work/