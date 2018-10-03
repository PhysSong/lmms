INCLUDE(BuildPlugin)

BUILD_PLUGIN(midioutinstr MidiOutInstr.cpp MidiOutInstr.h MOCFILES MidiOutInstr.h EMBEDDED_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.png")
