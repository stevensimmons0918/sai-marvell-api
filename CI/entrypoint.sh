#!/usr/bin/env bash
set -x

# Prepare docker image
docker build -t make_sai:jenkins . -f $PWD/CI/Dockerfile

# Run docker container
docker run --rm --workdir="/var/sai-marvell-api/" -v $PWD/:/var/sai-marvell-api --cap-add=NET_ADMIN --device=/dev/net/tun make_sai:jenkins ./CI/run_in_docker.sh 
