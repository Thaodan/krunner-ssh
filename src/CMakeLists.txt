set(krunner_ssh_SRCS
    ssh.cpp
)

add_library(krunner_ssh MODULE ${krunner_ssh_SRCS})
target_link_libraries(krunner_ssh
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
)

install(TARGETS krunner_ssh DESTINATION
  ${PLUGIN_INSTALL_DIR})
install(FILES core/emojis.json DESTINATION ${KDE_INSTALL_DATADIR}/emojirunner/)
