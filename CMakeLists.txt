cmake_minimum_required(VERSION 3.16.0)

project(KrunnerSSH)

set( QT_USE_QTXML True )

find_package(ECM 5.12.0 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH
  ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH}
)

# Find the required Libaries
find_package (Qt5 ${QT_MIN_VERSION}
  REQUIRED CONFIG COMPONENTS
  Widgets
  Core
  Xml
)
find_package (KF5 ${KF5_MIN_VERSION}
  REQUIRED COMPONENTS
  KIO
  I18n
  Service
  Runner
  Plasma
  CoreAddons
)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)


add_subdirectory(src)
