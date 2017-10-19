INCLUDE(BuildPlugin)

BUILD_PLUGIN(noyzr NoyzrEffect.cpp NoyzrControls.cpp NoyzrControlsDialog.cpp MOCFILES NoyzrControls.h NoyzrControlsDialog.h EMBEDDED_RESOURCES *.png)
