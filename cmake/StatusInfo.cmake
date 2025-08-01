message("===================================================")
message("Mangos revision       : ${rev_hash} ${rev_date} (${rev_branch} branch)")
message("Build type            : ${CMAKE_BUILD_TYPE}")
message("Install server(s) to  : ${BIN_DIR}")
message("Install configs to    : ${CONF_INSTALL_DIR}")

message("")
message("Detailed Information")
message("+-- operating system  : ${CMAKE_HOST_SYSTEM}")
message("+-- cmake version     : ${CMAKE_VERSION}")
message("")

if(BUILD_MANGOSD)
    message("Build main server     : Yes (default)")
    if(SCRIPT_LIB_ELUNA)
        message("+-- with Eluna script engine")
        message("    +-- revision      : ${dep_eluna_rev_hash} ${dep_eluna_rev_date} (${dep_eluna_rev_branch} branch)")
    endif()
    if(SCRIPT_LIB_SD3)
        message("+-- with SD3 script engine")
        message("    +-- revision      : ${dep_sd3_rev_hash} ${dep_sd3_rev_date} (${dep_sd3_rev_branch} branch)")
    endif()
    if(PLAYERBOTS)
        message("+-- with PlayerBots")
    endif()
else()
    message("Build main server     : No")
endif()

if(BUILD_REALMD)
    message("Build login server    : Yes (default)")
else()
    message("Build login server    : No")
endif()

if(SOAP)
    message("Support for SOAP      : Yes")
else()
    message("Support for SOAP      : No (default)")
endif()

if(BUILD_TOOLS)
    message("Build tools           : Yes (default)")
else()
    message("Build tools           : No")
endif()

if(WITHOUT_GIT)
  message("Use GIT revision hash   : No")
  message("")
  message(" *** WITHOUT_GIT - WARNING!")
  message(" *** Without Git installed, proper support may not be given.")
  message(" *** Some additional information that may be required, is only")
  message(" *** given when Git is enabled. Please install Git for support.")
else()
  message("Use GIT revision hash   : Yes (default)")
endif()

message("")
message("===================================================")
