#!/usr/bin/env bash

sudo add-apt-repository ppa:beineri/opt-qt597-xenial -y

sudo dpkg --add-architecture i386
sudo apt-get update -qq || true
