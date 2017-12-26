INCLUDE(BuildPlugin)

BUILD_PLUGIN(distenv DistEnvEffect.cpp DistEnvControls.cpp DistEnvControlDialog.cpp MOCFILES DistEnvControls.h DistEnvControlDialog.h EMBEDDED_RESOURCES artwork.png logo.png)
