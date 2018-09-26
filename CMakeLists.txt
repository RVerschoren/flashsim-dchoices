cmake_minimum_required(VERSION 3.2)

project(flashsim-dchoices)

#add_definitions("-x c++")
#add_definitions(-std=c++11)
set(OWN_GXX_FLAGS "-Wall -Wextra -std=c++11 -DNOT_USE_BLOCKMGR -DSINGLE_PLANE -DNO_NOOP -DNO_BLOCK_STATE -DNO_PLANE_STATE")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${OWN_GXX_FLAGS}") #" #-DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OWN_GXX_FLAGS} -Ofast")
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${OWN_GXX_FLAGS}" )

#set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${OWN_CXX_LINKER_FLAGS}" )

############################################################
# Create a library
############################################################
set(ssd_sources
    drivers/run_utils.cpp
    src/ssd_address.cpp
    src/ssd_block.cpp
    src/ssd_bm.cpp
    src/ssd_bus.cpp
    src/ssd_channel.cpp
    src/ssd_config.cpp
    src/ssd_controller.cpp
    src/ssd_die.cpp
    src/ssd_event.cpp
    src/ssd_eventreader.cpp
    #src/ssd_ftl.cpp
    src/ssd_ftlparent.cpp
    src/ssd_gc.cpp
    src/ssd_hcident.cpp
    src/ssd_package.cpp
    src/ssd_page.cpp
    src/ssd_plane.cpp
    src/ssd_raidssd.cpp
    src/ssd_ram.cpp
    src/ssd_ssd.cpp
    src/ssd_stats.cpp
    src/ssd_util.cpp
    src/ssd_wl.cpp
    #src/SSDSim.cpp
    src/FTLs/bast_ftl.cpp
    src/FTLs/bdftl_ftl.cpp
    src/FTLs/cold_ftl.cpp
    src/FTLs/dftl_ftl.cpp
    src/FTLs/dftl_parent.cpp
    src/FTLs/dwf_ftl.cpp
    src/FTLs/fast_ftl.cpp
    src/FTLs/hcwf_ftl.cpp
    src/FTLs/page_ftl.cpp
    src/FTLs/stat_ftl.cpp
    src/FTLs/swf_ftl.cpp
    src/GCAs/costagetime_gc.cpp
    src/GCAs/costbenefit_gc.cpp
    src/GCAs/dch_costagetime_gc.cpp
    src/GCAs/dch_costbenefit_gc.cpp
    src/GCAs/dchoices_gc.cpp
    src/GCAs/fifo_gc.cpp
    src/GCAs/greedy_gc.cpp
    src/GCAs/random_gc.cpp
    src/HCident/oracle_hcid.cpp
    src/HCident/static_hcid.cpp
    src/HCident/warm_hcid.cpp
    src/util/util_func.cpp
    src/util/util_rng.cpp
    src/WLvl/ban_prob_wlvl.cpp
    src/WLvl/ban_wlvl.cpp
    src/WLvl/cleanmaxvalid_wlvl.cpp
    src/WLvl/cleanmaxvaliddch_wlvl.cpp
    src/WLvl/hotcoldswap_wlvl.cpp
    src/WLvl/randomswap_wlvl.cpp
)

add_library(ssd_lib STATIC ${ssd_sources})
target_include_directories(ssd_lib
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

############################################################
# Create an executable
############################################################

# Source fles for the binary
#file(GLOB EXE_SOURCES RELATIVE drivers run_*.cpp)
set(EXE_SOURCES
	drivers/run_ComputeOracle.cpp
	drivers/run_ConvertBIOTrace.cpp
	drivers/run_SimBanProb.cpp
	drivers/run_SimHCSwap.cpp
	drivers/run_SimMaxValid.cpp
	drivers/run_SimNoWear.cpp
	drivers/run_SimSwap.cpp
	drivers/run_SimTraceBanProb.cpp
	drivers/run_SimTraceHCSwap.cpp
	drivers/run_SimTraceMaxValid.cpp
	drivers/run_SimTraceNoWear.cpp
	drivers/run_SimTraceSwap.cpp
	drivers/run_StatsBIOTrace.cpp
)

#message(STATUS "Executables")
foreach( exesourcefile ${EXE_SOURCES} )
    string( REPLACE ".cpp" ".exe" exename ${exesourcefile} )
    string( REPLACE "drivers/run_" "" exename ${exename} )
#    message(STATUS "${exename}")
    add_executable( ${exename} ${exesourcefile})
    target_link_libraries(${exename} ssd_lib)
    target_include_directories(${exename}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
    install( TARGETS ${exename} DESTINATION ${PROJECT_SOURCE_DIR}/bin )
    #add_custom_command(TARGET ${exename} PRE_BUILD
    #                   COMMAND ${CMAKE_COMMAND} -E copy
    #                       ${PROJECT_SOURCE_DIR}/ssd.conf $<TARGET_FILE_DIR:${exename}>)
    configure_file( ${PROJECT_SOURCE_DIR}/ssd.conf ssd.conf COPYONLY )
endforeach( exesourcefile ${EXE_SOURCES} )
configure_file( ${PROJECT_SOURCE_DIR}/ssd.conf ${PROJECT_SOURCE_DIR}/bin COPYONLY )
