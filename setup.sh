#!/bin/bash

function info()
{
    echo -e "\e[00;32m$*\e[00m" >&2 #green
}

function warn()
{
    echo -e "\e[00;35m$*\e[00m" >&2 #purple
}

function error()
{
    echo -e "\e[00;31m$*\e[00m" >&2 #green
    exit 1
}

# workspace setup
root=$(pwd)


# create external dir
external=$root/external
if [ ! -d $external ]; then
    mkdir -vp external
fi




# get gtest

#sudo apt install curl
#curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
#echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
#sudo apt update && sudo apt install bazel
gtest=$external/googletest
if [ ! -d $gtest ]; then
    cd $external
    git clone https://github.com/google/googletest.git
    if [ "$?" -ne "0" ]; then
        error "failed to clone gtest"
    fi

    cd $gtest
    bazel clean
    bazel build gtest
    bazel build gtest_main

    cd $root
else
    info "gtest already in place, skipping ..."
fi
echo


# check the machine architecture and grab cross-compilers
tools_arm="."
arch=$(uname --machine)
if [ $arch == "x86_64" ]; then
    info "you are using x86 ($arch) architecture"
    info "do you want to setup the cross compiler?"
    read -p "[y/n]" -n 1 -r
    echo    # new line
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo

        # tools for arm64
        tools_arm=$external/tools_arm

        # get cross-compile tools for arm 64
        if [ ! -d $tools_arm ]; then
            mkdir -p $tools_arm
            cd $tools_arm

            info "clone arm tools..."
            git clone https://github.com/raspberrypi/tools
            if [ "$?" -ne "0" ]; then
                error "failed to get arm tools"
            fi

            #info "checkout old but stable version ..."
            #cd $tools_arm/tools
            #git checkout 3a413ca2b23fd275e8ddcc34f3f9fc3a4dbc723f
            #if [ "$?" -ne "0" ]; then
            #    error "failed to checkout"
            #fi
        else
            info "arm tools already in place, skipping ..."
        fi

        info "create makefile.prefix_arm ..."
        cd $root
        echo "PREFIX=$tools_arm/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-" > ./makefile.prefix_arm
        echo "INC=-I$tools_arm/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/arm-linux-gnueabihf/include" >> ./makefile.prefix_arm
        echo "LIB=-L$tools_arm/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/arm-linux-gnueabihf/lib" >> ./makefile.prefix_arm
        echo "RPI_TARGET=yes" >> ./makefile.prefix_arm
        echo


        # tools for esp32
        tools_esp32=$external/tools_esp
        esp32_version=xtensa-esp-elf-gcc8_2_0-esp32-2019r2-linux-amd64.tar.gz

        # get cross-compile tools
        if [ ! -d $tools_esp32 ]; then
            cd $external

            wget -c https://dl.espressif.com/dl/$esp32_version
            if [ "$?" -ne "0" ]; then
                error "failed to download esp tools"
            fi

            mkdir -p $tools_esp32
            cd $tools_esp32
            tar -xvf $external/$esp32_version

            cd $external
            rm $esp32_version

            cd $tools_esp32
            info "clone esp idf ..."
            git clone --recursive https://github.com/espressif/esp-idf.git
            if [ "$?" -ne "0" ]; then
                error "failed to get esp idf"
            fi

        else
            info "esp tools already in place, skipping ..."
        fi

        info "create makefile.prefix_esp ..."
        cd $root
        echo "PREFIX=$tools_esp32/xtensa-esp32-elf/bin/xtensa-esp32-elf-" > ./makefile.prefix_esp
        echo "IDF_PATH=$tools_esp32/esp-idf" >> ./makefile.prefix_esp
        echo

    else
        echo "PREFIX=" > ./makefile.prefix_arm
        echo "PREFIX=" > ./makefile.prefix_esp
    fi


    echo "LINUX_TARGET=yes" > ./makefile.prefix_linux
fi
echo


boost=$external/boost_1_71_0
if [ ! -d $boost ]; then
    cd $external
    wget -c https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.tar.bz2
    if [ "$?" -ne "0" ]; then
        error "failed to get download boost"
    fi

    tar -xvf boost_1_71_0.tar.bz2
    rm boost_1_71_0.tar.bz2

    cd $boost
    ./bootstrap.sh
    echo "using gcc : : $tools_arm/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-g++ ; " > tools/build/src/user-config.jam
    ./b2 --no-cmake-config --without-python --without-mpi --without-graph_parallel variant=release link=static --prefix=$tools_arm/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/arm-linux-gnueabihf install
    cd $root
else
    info "boost already in place, skipping ..."
fi


# get cpp-json
jsonlib=$external/cpp-json
if [ ! -d $jsonlib ]; then
    cd $external
    git clone https://github.com/eteran/cpp-json.git
    if [ "$?" -ne "0" ]; then
        error "failed to get cpp-json"
    fi
    cd $root
else
    info "cpp-json already in place, skipping ..."
fi
echo


# get openssl
openssl=$external/openssl-1.1.0e
if [ ! -d $openssl ]; then
    cd $external
    wget -c https://www.openssl.org/source/openssl-1.1.0e.tar.gz
    if [ "$?" -ne "0" ]; then
        error "failed to get download openssl"
    fi

    tar -xzvf openssl-1.1.0e.tar.gz
    rm openssl-1.1.0e.tar.gz

    cd $openssl
    ./Configure --prefix=$openssl/install --cross-compile-prefix=$tools_arm/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf- no-zlib no-weak-ssl-ciphers no-unit-test no-md2 linux-armv4
    #linux-x86_64
    make all
    make install
    cd $root
else
    info "openssl already in place, skipping ..."
fi
echo


# get curl
curl=$external/curl-7.54.0
if [ ! -d $curl ]; then
    cd $external
    wget -c https://github.com/curl/curl/releases/download/curl-7_54_0/curl-7.54.0.tar.gz
    if [ "$?" -ne "0" ]; then
        error "failed to get download curl"
    fi

    tar -xzvf curl-7.54.0.tar.gz
    rm curl-7.54.0.tar.gz

    cd $curl
    export PATH=$PATH:$tools_arm/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin
    export CPPFLAGS="-I$tools_arm/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/include"
    export AR=arm-linux-gnueabihf-ar
    export AS=arm-linux-gnueabihf-as
    export LD=arm-linux-gnueabihf-ld
    export RANLIB=arm-linux-gnueabihf-ranlib
    export CC=arm-linux-gnueabihf-gcc
    export NM=arm-linux-gnueabihf-nm

    ./configure --host aarch64-linux --with-pic --disable-shared --without-zlib --without-librtmp --disable-ipv6 \
    --disable-unix-sockets --disable-smtp --disable-smb --disable-imap --disable-pop3 --disable-rtsp --disable-telnet \
    --disable-ldaps --disable-ldap --disable-ftp --disable-gopher --disable-dict --disable-tftp --disable-sspi \
    --without-winidn --without-libidn2 --without-nghttp2 --without-libmetalink --without-libpsl --without-nss \
    --disable-ntlm-wb --without-libssh2 --prefix=$curl/install --with-ssl=$openssl/install
    make all
    make install
    cd $root
else
    info "curl already in place, skipping ..."
fi
echo


exit 1
# get wiringPi
wiringPi=$external/wiringPi
if [ ! -d $wiringPi ]; then
    cd $external
    git clone git://git.drogon.net/wiringPi
    if [ "$?" -ne "0" ]; then
        error "failed to get wiringPi"
    fi

    echo
    info "process wiringPi ..."
    cd wiringPi/wiringPi

    cp $external/../wiringPi.patch .
    patch -p0 < wiringPi.patch

    make clean
    make static
    cd $root
else
    info "wiringPi already in place, skipping ..."
fi
