#!/bin/bash

function info()
{
    echo -e "\e[00;32m$*\e[00m" >&2 #green
}

if [ ! -e xelnaga_ami_data ]; then
    info "file xelnaga_ami_data does not exist. will be created with default options"

    touch xelnaga_ami_data
    echo AWS_HOST_CERT=xelnaga_ami_key.pem > xelnaga_ami_data
    echo AWS_HOST_NAME=localhost >> xelnaga_ami_data
    echo AWS_HOST_USER=ubuntu >> xelnaga_ami_data
    echo AWS_HOST_PATH=/home/ubuntu >> xelnaga_ami_data
fi

. xelnaga_ami_data

function copy_to_aws()
{
    FILE_NAME=$*
    scp -i $AWS_HOST_CERT $FILE_NAME $AWS_HOST_USER@$AWS_HOST_NAME:$AWS_HOST_PATH/$FILE_NAME
}

echo
info "copy file to aws"
copy_to_aws shp_server
copy_to_aws service-account.json

echo
info "use command to connect to the server:"
echo "ssh -v -i $AWS_HOST_CERT $AWS_HOST_USER@$AWS_HOST_NAME"
