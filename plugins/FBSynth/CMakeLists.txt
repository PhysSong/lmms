INCLUDE(BuildPlugin)
BUILD_PLUGIN(fbsynth
	FBSynthInstrument.cpp FBSynthInstrumentView.cpp FBSynthCurve.cpp FBSynthParameterWidget.cpp FBFilterInstance.cpp
	MOCFILES FBSynthInstrumentView.h FBSynthCurve.h FBSynthParameterWidget.h
	EMBEDDED_RESOURCES *.png
)
