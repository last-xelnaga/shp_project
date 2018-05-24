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
    ./Configure --prefix=$openssl/install no-zlib no-weak-ssl-ciphers no-unit-test no-md2 linux-x86_64
    make all
    make install
    cd $root
else
    info "openssl already in place, skipping ..."
fi


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
    ./configure --enable-shared=no --without-zlib --without-librtmp --disable-ipv6 --disable-unix-sockets \
    --disable-smtp --disable-smb --disable-imap --disable-pop3 --disable-rtsp --disable-telnet --disable-ldaps \
    --disable-ldap --disable-ftp --disable-gopher --disable-dict --disable-tftp --with-ssl=$openssl/install
    make all
    cd $root
else
    info "curl already in place, skipping ..."
fi


# check the machine architecture
tools=$external/tools
arch=$(uname --machine)
if [ $arch == "x86_64" ]; then
    info "you are using x86 ($arch) architecture"
    info "do you want to setup the cross compiler?"
    read -p "[y/n]" -n 1 -r
    echo    # new line
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo
        # get cross-compile tools
        if [ ! -d $tools ]; then
            cd $external

            info "clone tools..."
            git clone https://github.com/raspberrypi/tools
            if [ "$?" -ne "0" ]; then
                error "failed to get tools"
            fi

            info "checkout old but stable version ..."
            cd $tools
            git checkout 3a413ca2b23fd275e8ddcc34f3f9fc3a4dbc723f
            if [ "$?" -ne "0" ]; then
                error "failed to checkout"
            fi
        else
            info "tools already in place, skipping ..."
        fi

        info "create makefile.prefix ..."
        cd $root
        echo "PREFIX = $tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-" > ./client/makefile.prefix
        echo "WIRING_LIB := yes" >> ./client/makefile.prefix
    else
        echo "PREFIX = " > ./client/makefile.prefix
    fi
fi


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
