
THEOS_DEVICE_IP = 192.168.1.36
#THEOS_DEVICE_IP = 192.168.1.77
#THEOS_DEVICE_IP = 192.168.1.215
THEOS_DEVICE_PORT=22

ARCHS = arm64 

DEBUG = 0
FINALPACKAGE = 1
FOR_RELEASE = 1

TARGET = iphone:clang:latest:latest
CFLAGS = -fobjc-arc

include $(THEOS)/makefiles/common.mk

TWEAK_NAME = LastIsland




LastIsland_FRAMEWORKS =  UIKit Foundation Security QuartzCore CoreGraphics CoreText  AVFoundation Accelerate GLKit SystemConfiguration GameController

LastIsland_CCFLAGS = -std=c++17 -fno-rtti -fno-exceptions -DNDEBUG -fvisibility=hidden
LastIsland_CFLAGS = -fobjc-arc -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-value -fvisibility=hidden 

LastIsland_FILES = ImGuiDrawView.xm CheatState.cpp $(wildcard Esp/*.mm)   $(wildcard Esp/*.m) $(wildcard KittyMemory/*.cpp) $(wildcard KittyMemory/*.mm) $(wildcard img/*.m)



#LQMNemOS_LIBRARIES += substrate
# GO_EASY_ON_ME = 1

include $(THEOS_MAKE_PATH)/tweak.mk
include $(THEOS)/makefiles/aggregate.mk
after-install::
   install.exec "killall -9 kgvn || :"


