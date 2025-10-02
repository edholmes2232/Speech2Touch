# Using Ubuntu image contents, see here for details: https://github.com/devcontainers/images/tree/main/src/base-ubuntu
# Select Ubuntu version, options are "20.04" (Focal), "22.04" (Jammy) or "24.04" (noble)
ARG VARIANT="24.04"

FROM mcr.microsoft.com/devcontainers/base:ubuntu-${VARIANT}

# Prevent interactive prompts during package installation
ARG DEBIAN_FRONTEND=noninteractive

# Install required packages to build firmware and documentation
RUN apt-get update \
    && apt-get -y install --no-install-recommends \
        software-properties-common \
        build-essential \
        git \
        symlinks \
        expect \
        bash-completion \
        cmake \
        ninja-build \
        wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Download and install the Arm GNU toolchain
ARG TOOLCHAIN_VER="12.2.rel1"
ARG TOOLCHAIN_URL="https://developer.arm.com/-/media/Files/downloads/gnu/${TOOLCHAIN_VER}/binrel/arm-gnu-toolchain-${TOOLCHAIN_VER}-x86_64-arm-none-eabi.tar.xz"

RUN wget -q $TOOLCHAIN_URL -O /tmp/arm-gnu-toolchain.tar.xz \
    && mkdir -p /opt/arm-gnu-toolchain \
    && tar -xf /tmp/arm-gnu-toolchain.tar.xz -C /opt/arm-gnu-toolchain --strip-components=1 \
    && rm /tmp/arm-gnu-toolchain.tar.xz \
    && for tool in gcc gdb g++ size objcopy objdump nm c++filt; do \
        ln -s /opt/arm-gnu-toolchain/bin/arm-none-eabi-${tool} /usr/bin/arm-none-eabi-${TOOLCHAIN_VER}-${tool}; \
    done

# Add toolchain to PATH
ENV PATH="/opt/arm-gnu-toolchain/bin:${PATH}"
