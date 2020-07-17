#!/usr/bin/env bash

PACKAGES="cmake pkgconfig fftw libogg libvorbis lame libsndfile libsamplerate jack sdl libgig libsoundio stk fluid-synth portaudio node fltk carla"

brew install $PACKAGES ccache

if [ $QT5 ]; then
	brew install qt
fi


if [ -z "$QT5" ]; then
	brew tap cartr/qt4
	brew tap-pin cartr/qt4
	brew install qt@4
fi

npm install -g appdmg
