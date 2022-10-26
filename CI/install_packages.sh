!#/usr/bin/bash

apt-get update
apt-get install -y net-tools libxml2 libxml2-dev libpcap0.8 libpcap-dev swig doxygen \
    xsltproc python-dev python-pip python-pip python-pexpect unifdef gcc-5 g++-5
pip install scapy==2.4.3
update-alternatives --remove-all gcc
update-alternatives --remove-all g++
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 50
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 50
update-alternatives --install /usr/bin/gcc-ar gcc-ar /usr/bin/gcc-ar-5 50
update-alternatives --install /usr/bin/gcc-nm gcc-nm /usr/bin/gcc-nm-5 50
update-alternatives --install /usr/bin/gcc-ranlib gcc-ranlib /usr/bin/gcc-ranlib-5 50
