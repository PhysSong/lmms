#!/usr/bin/env bash

PACKAGES="cmake pkgconfig fftw libogg libvorbis lame libsndfile libsamplerate jack sdl libgig libsoundio stk fluid-synth portaudio node fltk carla"

brew install $PACKAGES ccache

if [ $QT5 ]; then
	# Try Qt 5.13.2 instead of the latest
	brew reinstall "https://raw.githubusercontent.com/Homebrew/homebrew-core/65a45a9e61f15046a256cdba6a008f38f79570c9/Formula/qt.rb"
fi


if [ -z "$QT5" ]; then
	brew tap cartr/qt4
	brew tap-pin cartr/qt4
	brew install qt@4
fi

npm install -g appdmg
