set(ssh_SRCS
    ssh.cpp
)

add_library(ssh MODULE ${krunner_ssh_SRCS})
target_link_libraries(ssh
  Qt5::Widgets
  Qt5::Xml
  KF5::CoreAddons
  KF5::Runner
  KF5::Plasma
  KF5::I18n
  KF5::Service
  KF5::KIOCore
  KF5::KIOFileWidgets
  KF5::KIOWidgets
  KF5::KIONTLM
) # Widgets for QAction, this will change to Qt::Gui in Qt6

install(TARGETS ssh DESTINATION
  ${PLUGIN_INSTALL_DIR})
install(FILES core/emojis.json DESTINATION ${KDE_INSTALL_DATADIR}/emojirunner/)
