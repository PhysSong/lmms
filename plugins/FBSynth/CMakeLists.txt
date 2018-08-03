INCLUDE(BuildPlugin)
BUILD_PLUGIN(fbsynth FBSynthInstrument.cpp FBSynthCurve.cpp FBSynthCurve.h FBSynthInstrumentView.cpp FBSynthParameterWidget.cpp
FBSynthInstrument.h FBSynthFilter.h FBSynthFader.h FBSynthInstrumentView.h FBSynthCurve.h FBSynthParameterWidget.h FBSynthFader.h
MOCFILES FBSynthInstrumentView.h FBSynthCurve.h FBSynthParameterWidget.h EMBEDDED_RESOURCES *.png)
