FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install required packages
RUN apt-get update && apt-get install -y \
    mpich \
    openssh-server \
    sshpass \
    sudo \
    openssl \
    && mkdir -p /var/run/sshd

# Create the 'vagrant' user and grant sudo without password
RUN useradd -m -s /bin/bash -p $(openssl passwd -1 vagrant) vagrant && \
    echo "vagrant ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Switch to vagrant user and set up SSH
USER vagrant
WORKDIR /home/vagrant

RUN mkdir -p /home/vagrant/.ssh && \
    ssh-keygen -t rsa -N "" -f /home/vagrant/.ssh/id_rsa && \
    cat /home/vagrant/.ssh/id_rsa.pub >> /home/vagrant/.ssh/authorized_keys && \
    chmod 600 /home/vagrant/.ssh/authorized_keys && \
    chmod 700 /home/vagrant/.ssh

# Add hosts to known_hosts
RUN ssh-keyscan -H mpi-node1 >> /home/vagrant/.ssh/known_hosts 2>/dev/null || true && \
    ssh-keyscan -H mpi-node2 >> /home/vagrant/.ssh/known_hosts 2>/dev/null || true && \
    ssh-keyscan -H mpi-node3 >> /home/vagrant/.ssh/known_hosts 2>/dev/null || true && \
    ssh-keyscan -H mpi-node4 >> /home/vagrant/.ssh/known_hosts 2>/dev/null || true

# Set correct ownership
USER root
RUN chown -R vagrant:vagrant /home/vagrant/.ssh

# Expose SSH port
EXPOSE 22

# Start SSH service and keep the container running
CMD service ssh start && tail -f /dev/null
