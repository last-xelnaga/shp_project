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
external=$root/external
jsonlib=$external/cpp-json
wiringPi=$external/wiringPi
tools=$external/tools

# create external dir
if [ ! -d $external ]; then
    mkdir -vp external
fi

# get cpp-json
if [ ! -d $jsonlib ]; then
    cd $external
    git clone https://github.com/eteran/cpp-json.git
    if [ "$?" -ne "0" ]; then
        error "failed to get cpp-json"
    fi
    cd $root
else
    info "cpp-json already in place, skipping..."
fi


# check the machine architecture
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

            info "checkout old but stable version..."
            cd $tools
            git checkout 3a413ca2b23fd275e8ddcc34f3f9fc3a4dbc723f
            if [ "$?" -ne "0" ]; then
                error "failed to checkout"
            fi
        else
            info "tools already in place, skipping..."
        fi

        info "create makefile.prefix ..."
        cd $root
        echo "PREFIX = $tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-" > ./client/makefile.prefix
    else
        echo "PREFIX = " > ./client/makefile.prefix
    fi
fi

# get wiringPi
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
    info "wiringPi already in place, skipping..."
fi
