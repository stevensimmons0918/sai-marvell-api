#!/usr/bin/env bash
set -x

# Run docker container
docker run --sysctl net.ipv6.conf.all.disable_ipv6=1 --rm --workdir="/var/sai-marvell-api/" -v $PWD/:/var/sai-marvell-api --cap-add=NET_ADMIN --device=/dev/net/tun make_sai:jenkins ./CI/run_sanity_tests.sh
