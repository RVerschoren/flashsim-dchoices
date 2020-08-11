cmake_minimum_required(VERSION 2.8)

project(flashsim-dchoices)

set (CMAKE_CXX_STANDARD 17)
#add_definitions("-x c++")
#add_definitions(-std=c++11)
# c:\PROGRA~2\MICROS~1\2019\COMMUN~1\VC\Tools\Llvm\bin\clang-cl.exe  /nologo -TP  -I..\..\..\include -m64 -fdiagnostics-absolute-paths  /DWIN32 /D_WINDOWS /W3 /GR /EHsc /Wall /std:c++14 /DNOT_USE_BLOCKMGR /DNO_NOOP /DNO_BLOCK_STATE /DNO_PLANE_STATE /MD /Zi /O2 /Ob1 /DNDEBUG /showIncludes /FoCMakeFiles\ssd_lib.dir\src\ssd_address.cpp.obj /FdCMakeFiles\ssd_lib.dir\ssd_lib.pdb -c ..\..\..\src\ssd_address.cpp
 
if(WIN32) #MSVC for VC compiler 
	set(OWN_CXX_FLAGS "/Wall /std:c++latest  /DNOT_USE_BLOCKMGR /DNO_NOOP /DNO_BLOCK_STATE /DNO_PLANE_STATE")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${OWN_CXX_FLAGS} /DEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${OWN_CXX_FLAGS} /O2 /DEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OWN_CXX_FLAGS} /O2 /DNDEBUG")
else()
	set(OWN_CXX_FLAGS "-Wall -Wextra  -DNOT_USE_BLOCKMGR -DNO_NOOP -DNO_BLOCK_STATE -DNO_PLANE_STATE")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${OWN_CXX_FLAGS} -g -pg")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${OWN_CXX_FLAGS} -O2 -g -pg")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OWN_CXX_FLAGS} -O3 -DNDEBUG")
endif()
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${OWN_CXX_FLAGS}" )

#set(MATLAB_LIBS_DIRECTORY ${MATLAB_ROOT}/bin/win64)
#SET(CMAKE_FIND_LIBRARY_PREFIXES lib )
#SET(CMAKE_FIND_LIBRARY_SUFFIXES .dll.a .a )

#Optional for basename implementation
#set(OWN_CXX_LINKER_FLAGS "-lboost_filesystem -lboost_system")
#set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${OWN_CXX_LINKER_FLAGS}" )

############################################################
# Create a library
############################################################
set(ssd_sources
    drivers/run_utils.cpp
    src/ssd_address.cpp
    src/ssd_block.cpp
#   src/ssd_bm.cpp
    src/ssd_bus.cpp
    src/ssd_channel.cpp
    src/ssd_config.cpp
    src/ssd_controller.cpp
    src/ssd_die.cpp
    src/ssd_event.cpp
    src/ssd_eventreader.cpp
#   src/ssd_ftl.cpp
    src/ssd_ftlparent.cpp
    src/ssd_gc.cpp
    src/ssd_hcident.cpp
    src/ssd_package.cpp
    src/ssd_page.cpp
    src/ssd_plane.cpp
    src/ssd_planeaddress.cpp
    src/ssd_raidssd.cpp
    src/ssd_ram.cpp
    src/ssd_ssd.cpp
    src/ssd_stats.cpp
    src/ssd_util.cpp
    src/ssd_wl.cpp
#   src/SSDSim.cpp
#   src/FTLs/bast_ftl.cpp
#   src/FTLs/bdftl_ftl.cpp
    src/FTLs/cold_ftl.cpp
    src/FTLs/cold_dHC_ftl.cpp
#   src/FTLs/dftl_ftl.cpp
#   src/FTLs/dftl_parent.cpp
    src/FTLs/dwf_ftl.cpp
    src/FTLs/dwfwdiff_ftl.cpp
    src/FTLs/dswapwf_ftl.cpp
    src/FTLs/fast_ftl.cpp
    src/FTLs/hcwf_ftl.cpp
    src/FTLs/hcwfwdiff_ftl.cpp
    src/FTLs/hcwf_dHC_ftl.cpp
    src/FTLs/hcswapwf_ftl.cpp
    src/FTLs/hcswapwf_hctie_ftl.cpp
    src/FTLs/hcwfswap_erasetie_ftl.cpp
    src/FTLs/hcwfswap_erase.cpp
    src/FTLs/hcwfswap_erasetie_ftl.cpp
    src/FTLs/hcwfswap_erase_validtie.cpp
    src/FTLs/hcwffalse_ftl.cpp
    src/FTLs/hcwffixedfalse_ftl.cpp
    src/FTLs/hcwfplusswap_ftl.cpp
    src/FTLs/page_ftl.cpp
    src/FTLs/stat_ftl.cpp
    src/FTLs/swf_ftl.cpp
		src/FTLs/swfwdiff_ftl.cpp
		src/GCAs/costagetime_gc.cpp
    src/GCAs/costbenefit_gc.cpp
    src/GCAs/dch_costagetime_gc.cpp
    src/GCAs/dch_costbenefit_gc.cpp
    src/GCAs/dch_tieerasures_gc.cpp
    src/GCAs/dch_erase_tievalidpages_gc.cpp
    src/GCAs/dch_erase_gc.cpp
    src/GCAs/dch_hotcold_gc.cpp
    src/GCAs/dchoices_gc.cpp
    src/GCAs/fifo_gc.cpp
    src/GCAs/greedy_gc.cpp
    src/GCAs/random_gc.cpp
    src/HCident/oracle_hcid.cpp
    src/HCident/static_hcid.cpp
    src/HCident/static_false_hcid.cpp
    src/HCident/warm_hcid.cpp
    src/util/util_func.cpp
    src/util/util_rng.cpp
    src/WLvl/ban_prob_wlvl.cpp
    src/WLvl/ban_wlvl.cpp
    src/WLvl/cleanmaxvalid_wlvl.cpp
    src/WLvl/cleanmaxvaliddch_wlvl.cpp
    src/WLvl/hotcoldswap_wlvl.cpp
    src/WLvl/hotcoldgcswap_wlvl.cpp
    src/WLvl/hotcoldgcswap_erase_wlvl.cpp
    src/WLvl/randomswap_wlvl.cpp
)

set(ssd_lib_INCLUDE ${PROJECT_SOURCE_DIR}/include)
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
	drivers/run_SSD_Synth.cpp
        drivers/run_SimBanProb.cpp
        drivers/run_SimHCSwap.cpp
        drivers/run_SimHCGCSwap.cpp
	drivers/run_SimHCGCSwap_Erase.cpp
        drivers/run_SimHCSwapRelative.cpp
        drivers/run_SimMaxValid.cpp
        drivers/run_SimNoWear.cpp
        drivers/run_SimSwap.cpp
        drivers/run_SimTraceBanProb.cpp
        drivers/run_SimTraceHCSwap.cpp
        drivers/run_SimTraceHCGCSwap.cpp
	drivers/run_SimTraceHCGCSwap_Erase.cpp
        drivers/run_SimTraceHCSwapRelative.cpp
        drivers/run_SimTraceMaxValid.cpp
        drivers/run_SimTraceNoWear.cpp
        drivers/run_SimTraceSwap.cpp
        drivers/run_StatsBIOTrace.cpp
)

#message(STATUS "Executables")
foreach( exesourcefile ${EXE_SOURCES} )
    string( REPLACE ".cpp" ".exe" exename ${exesourcefile} )
    string( REPLACE "drivers/run_" "" exename ${exename} )
    message(STATUS "${exename}")
    add_executable( ${exename} ${exesourcefile})
    target_link_libraries(${exename} ssd_lib)
    target_include_directories(${exename}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
	set_target_properties(${exename} PROPERTIES COMPILE_FLAGS -DBUILDER_STATIC_DEFINE)
    #install( TARGETS ${exename} DESTINATION ${PROJECT_SOURCE_DIR}/bin )
    install( TARGETS ${exename} DESTINATION bin )
    # Copy for builds
    configure_file( ${PROJECT_SOURCE_DIR}/ssd.conf ssd.conf COPYONLY )
endforeach( exesourcefile ${EXE_SOURCES} )
# Copy for install
install(FILES ssd.conf DESTINATION bin)
#configure_file( ${PROJECT_SOURCE_DIR}/ssd.conf ${PROJECT_SOURCE_DIR}/bin COPYONLY )

if(WIN32)
	find_library(MATLAB_MEX_LIB mex 
		PATHS ${MATLAB_LIBS_DIRECTORY}
		NO_DEFAULT_PATH)
else()
	find_package(Matlab)
endif()
if(${Matlab_FOUND})
        # Make it work nicely with mex
        #message(STATUS ${Matlab_MEX_LIBRARY})
        set(MEX_INSTALL_DIR $ENV{HOME}/Projects/MATLAB)
        set(Matlab_INCLUDE_DIRS ${Matlab_INCLUDE_DIRS} ${ssd_lib_INCLUDE})
        matlab_add_mex(NAME mex_ssd
                SHARED
                #MODULE
                #EXECUTABLE
                SRC
                        #src/mex/mex_ssd.cpp
                        src/mex/mex_ssd.c
                        src/mex/mex_ssd_iface.cpp
                        ${ssd_sources}
                #DOCUMENTATION "src/mex/mex_ssd_doc.txt"
                LINK_TO ${Matlab_MEX_LIBRARY}
                )

        message(STATUS "Installing mex files to ${MEX_INSTALL_DIR}")
        install( TARGETS mex_ssd DESTINATION ${MEX_INSTALL_DIR})
        configure_file( ${PROJECT_SOURCE_DIR}/ssd.conf ${MEX_INSTALL_DIR}/ssd.conf COPYONLY )
endif(${Matlab_FOUND})
