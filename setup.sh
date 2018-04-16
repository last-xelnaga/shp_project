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
