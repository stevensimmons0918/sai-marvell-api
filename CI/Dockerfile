# Base Image
FROM ubuntu:18.04

# Update APT repository & Install packages
RUN apt-get update \
    && apt-get install -y net-tools libxml2 libxml2-dev libpcap0.8 libpcap-dev swig \ 
    && apt-get install -y doxygen xsltproc python-dev python-pip python-pip \
    && apt-get install -y ssh python-pexpect unifdef gcc-5 g++-5

RUN pip install scapy==2.4.3
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 50
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 50
RUN update-alternatives --install /usr/bin/gcc-ar gcc-ar /usr/bin/gcc-ar-5 50
RUN update-alternatives --install /usr/bin/gcc-nm gcc-nm /usr/bin/gcc-nm-5 50
RUN update-alternatives --install /usr/bin/gcc-ranlib gcc-ranlib /usr/bin/gcc-ranlib-5 50

# Establish the operating directory of sshd
RUN mkdir /var/run/sshd

# Set Root password
RUN echo 'root:password' | chpasswd

# Disable PAM Authentication
RUN sed -i 's/PAM yes/PAM no/' /etc/ssh/sshd_config

# Allow Root login
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' \
    /etc/ssh/sshd_config

# SSH login fix
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional \
    pam_loginuid.so@g' -i /etc/pam.d/sshd
