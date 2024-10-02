FROM mcr.microsoft.com/devcontainers/cpp:1-debian-12

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
  chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
  fi \
  && rm -f /tmp/reinstall-cmake.sh

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install <your-port-name-here>"

# [Optional] Uncomment this section to install additional packages.
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#     && apt-get -y install --no-install-recommends <your-package-list-here>

# Install OpenMPI library
# Building from source https://docs.open-mpi.org/en/v5.0.x/installing-open-mpi/quickstart.html#building-from-source
ARG USER="vscode"
ARG OPENMPI_VERSION="5.0.5"

USER $USER

WORKDIR /home/$USER
RUN wget https://download.open-mpi.org/release/open-mpi/v5.0/openmpi-$OPENMPI_VERSION.tar.bz2
RUN tar xf openmpi-$OPENMPI_VERSION.tar.bz2

WORKDIR /home/$USER/openmpi-${OPENMPI_VERSION}
RUN ./configure --prefix="/home/$USER/.openmpi" 2>&1 | tee config.out
RUN make all 2>&1 | tee make.out
RUN make install 2>&1 | tee install.out

RUN echo export PATH="$PATH:/home/$USER/.openmpi/bin" >> /home/$USER/.bashrc
RUN echo export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/$USER/.openmpi/lib/" >> /home/$USER/.bashrc