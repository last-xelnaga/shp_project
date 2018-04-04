#!/bin/bash

function info()
{
    echo -e "\e[00;32m$*\e[00m" >&2 #green
}

function error()
{
    echo -e "\e[00;31m$*\e[00m" >&2 #red
    exit 1
}


info "compile test apk"
if [ ! -d "$ANDROID_HOME" ]; then
    #echo $ANDROID_HOME
    info "ANDROID_HOME is not valid, check local.properties"

    SDK_PATH=$(cat local.properties | grep sdk.dir | sed 's/sdk.dir=//g')
    #echo $SDK_PATH
    if [ ! -d "$SDK_PATH" ]; then
        error "local.properties is not valid. no SDK"
    fi
fi

info "compile test apk"
./gradlew -b build.gradle clean assembleRelease
if [ "$?" -ne "0" ]; then
    error "gradlew failed"
fi

#info "install apk"
#adb wait-for-device install -r .apk
#if [ "$?" -ne "0" ]; then
#    error "installation failed"
#fi
