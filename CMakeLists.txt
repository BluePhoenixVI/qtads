cmake_minimum_required(VERSION 3.21)
set(VCPKG_TARGET_TRIPLET x64-windows-static)
project(qtads VERSION "3.4" LANGUAGES C CXX)

# Audio is unsupported, for now
set(DISABLEAUDIO ON)

# Global flag for enabling the dap extensions
set(ENABLE_DAP ON)

# Use a toolmake chain file
set(CMAKE_TOOLCHAIN_FILE on)

# Setup the C and C++ language versions, set them as required, and disable compiler language extensions
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Disable all deprecation warnings. TADS 3 is old code, and predates them.
set(CMAKE_WARN_DEPRECATED FALSE)

# Instruct cmake to use moc, rcc, and uic for targets that require them
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
# Include the qrc file so qt5 can find the QTads icon
set(APP_ICON_RESOURCE_WINDOWS "resources.qrc")

# Use the non-dll runtime, with a generator expression to select the debug version for debug builds
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# ----------------------------------------------------------
# Packages
# ----------------------------
# Find Qt5
find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Network Widgets)
# Pull in google/cppdap and nlohmann/json if we are including the dap interface
if(ENABLE_DAP)
  ## Do not use the FetchContent module to get the cppdat project, as the cppdat project
  ## depends on the nlohmann/json project and it will always unes the LATEST version.
  ## As we can only target a fixed version of the cppdat project using the the FetchContent
  ## module, we run the risk of the build breaking if/when the nlohmann/json project becomes
  ## imcompatable with the cppdat project.
  ## Instead, we simply directly include a copy of both projects.


  # The FetchContent module is needed to pull in the google/cppdap project
  include(FetchContent)
  # Pull in google/cppdap
  FetchContent_Declare(google_cppdap
    GIT_REPOSITORY https://github.com/google/cppdap.git
    GIT_TAG        252b56807b532533ea7362a4d949758dcb481d2b
  )
  # Tell google_cppdap to use an external copy of nlohmann_json rather then
  # the one it includes, as we can control which commit the external version
  # is from. The one google_cppdap will always be the latest, even if it cause
  # problems.
  set(CPPDAP_USE_EXTERNAL_NLOHMANN_JSON_PACKAGE YES)
  # A static reference to nlohmann/json. Use OVERRIDE_FIND_PACKAGE so that
  # when cppdap calls find_package to get nlohmann_json, it can find our
  # copy of it
  #FetchContent_Declare(nlohmann_json
  #  GIT_REPOSITORY https://github.com/nlohmann/json.git
  #  GIT_TAG        bc889afb4c5bf1c0d8ee29ef35eaaf4c8bef8a5d
  #  OVERRIDE_FIND_PACKAGE
  #)
  #FetchContent_MakeAvailable(nlohmann_json google_cppdap)
  FetchContent_MakeAvailable(google_cppdap)
endif()



# ------------------------------------------------------------------------------
# Add ALL sources needed to compile QTads.
# For now, they are in one big block.
add_executable(qtads WIN32 
    ${UI_SOURCES}
    ${HDR_SOURCES}
    htmltads/htmldisp.cpp
    htmltads/htmlfmt.cpp
    htmltads/htmlhash.cpp
    htmltads/htmlinp.cpp
    htmltads/htmlprs.cpp
    htmltads/htmlrc.cpp
    htmltads/htmlrf.cpp
    htmltads/htmlsnd.cpp
    htmltads/htmlsys.cpp
    htmltads/htmltags.cpp
    htmltads/htmltxar.cpp
    htmltads/oshtml.cpp
    htmltads/tadshtml.cpp
    htmltads/tadsrtyp.cpp
    src/aboutqtadsdialog.cc src/aboutqtadsdialog.h src/aboutqtadsdialog.ui
    src/confdialog.cc src/confdialog.h src/confdialog.ui
    src/config.h
    src/dispwidget.cc src/dispwidget.h
    src/dispwidgetinput.cc src/dispwidgetinput.h
    src/gameinfodialog.cc src/gameinfodialog.h src/gameinfodialog.ui
    src/globals.cc src/globals.h
    src/hos_qt.cc src/hos_qt.h
    src/hos_w32.h
    src/kcolorbutton.cc src/kcolorbutton.h
    src/main.cc
    src/missing.cc src/missing.h
    src/oemqt.c
    src/osqt.cc src/osqt.h
    src/oswin.h
    src/qtadshostifc.cc src/qtadshostifc.h
    src/qtadsimage.cc src/qtadsimage.h
    src/qtadssound.cc src/qtadssound.h
    src/qtadstimer.cc src/qtadstimer.h
    src/qtimerchrono.cc src/qtimerchrono.h
    src/settings.cc src/settings.h
    src/sysfont.cc src/sysfont.h
    src/sysframe.cc src/sysframe.h
    src/sysimagejpeg.cc src/sysimagejpeg.h
    src/sysimagemng.cc src/sysimagemng.h
    src/sysimagepng.cc src/sysimagepng.h
    src/syssoundmidi.cc src/syssoundmidi.h
    src/syssoundmpeg.cc src/syssoundmpeg.h
    src/syssoundogg.cc src/syssoundogg.h
    src/syssoundwav.cc src/syssoundwav.h
    src/syswin.cc src/syswin.h
    src/syswinaboutbox.cc src/syswinaboutbox.h
    src/syswingroup.cc src/syswingroup.h
    src/syswininput.cc src/syswininput.h
    src/util.cc src/util.h
    tads2/argize.c
    tads2/askf_os.c
    tads2/bif.c
    tads2/bifgdum.c
    tads2/cmap.c
    tads2/cmd.c
    tads2/dat.c
    tads2/dbgtr.c
    tads2/errmsg.c
    tads2/execmd.c
    tads2/fio.c
    tads2/fioxor.c
    tads2/getstr.c
    tads2/indlg_os.c
    tads2/ler.c
    tads2/linfdum.c
    tads2/lst.c
    tads2/mch.c
    tads2/mcm.c
    tads2/mcs.c
    tads2/obj.c
    tads2/oserr.c
    tads2/osifc.c
    tads2/osnoui.c
    tads2/osrestad.c
    tads2/osstzprs.c
    tads2/out.c
    tads2/output.c
    tads2/ply.c
    tads2/qas.c
    tads2/regex.c
    tads2/run.c
    tads2/runstat.c
    tads2/suprun.c
    tads2/trd.c
    tads2/voc.c
    tads2/vocab.c
    tads3/askf_os3.cpp
    tads3/charmap.cpp
    tads3/derived/vmuni_cs.cpp
    tads3/gameinfo.cpp
    tads3/indlg_os3.cpp
    tads3/md5.cpp
    tads3/resfind.cpp
    tads3/resload.cpp
    tads3/resnoexe.cpp
    tads3/sha2.cpp
    tads3/std.cpp
    tads3/tcerr.cpp
    tads3/tcerrmsg.cpp
    tads3/tcgen.cpp
    tads3/tcglob.cpp
    tads3/tcmain.cpp
    tads3/tcprs.cpp
    tads3/tcprs_rt.cpp
    tads3/tcprsnf.cpp
    tads3/tcprsnl.cpp
    tads3/tcprsstm.cpp
    tads3/tcsrc.cpp
    tads3/tct3.cpp
    tads3/tct3_d.cpp
    tads3/tct3nl.cpp
    tads3/tct3stm.cpp
    tads3/tct3unas.cpp
    tads3/tctok.cpp
    tads3/utf8.cpp
    tads3/vmanonfn.cpp
    tads3/vmbif.cpp
    tads3/vmbifl.cpp
    tads3/vmbifregx.cpp
    tads3/vmbift3.cpp
    tads3/vmbiftad.cpp
    tads3/vmbiftio.cpp
    tads3/vmbiftix.cpp
    tads3/vmbignum.cpp
    tads3/vmbignumlib.cpp
    tads3/vmbytarr.cpp
    tads3/vmcfgfl.cpp
    tads3/vmcoll.cpp
    tads3/vmconhmp.cpp
    tads3/vmconhtm.cpp
    tads3/vmconsol.cpp
    tads3/vmcrc.cpp
    tads3/vmcset.cpp
    tads3/vmdate.cpp
    tads3/vmdict.cpp
    tads3/vmdynfunc.cpp
    tads3/vmerr.cpp
    tads3/vmerrmsg.cpp
    tads3/vmfile.cpp
    tads3/vmfilnam.cpp
    tads3/vmfilobj.cpp
    tads3/vmfref.cpp
    tads3/vmfunc.cpp
    tads3/vmglob.cpp
    tads3/vmgram.cpp
    tads3/vmhash.cpp
    tads3/vmimage.cpp
    tads3/vminit.cpp
    tads3/vminitfl.cpp
    tads3/vmintcls.cpp
    tads3/vmisaac.cpp
    tads3/vmiter.cpp
    tads3/vmlog.cpp
    tads3/vmlookup.cpp
    tads3/vmlst.cpp
    tads3/vmmain.cpp
    tads3/vmmcreg.cpp
    tads3/vmmeta.cpp
    tads3/vmnetfillcl.cpp
    tads3/vmobj.cpp
    tads3/vmop.cpp
    tads3/vmpack.cpp
    tads3/vmpat.cpp
    tads3/vmpool.cpp
    tads3/vmpoolfl.cpp
    tads3/vmregex.cpp
    tads3/vmrun.cpp
    tads3/vmrunsym.cpp
    tads3/vmsa.cpp
    tads3/vmsave.cpp
    tads3/vmsort.cpp
    tads3/vmsortv.cpp
    tads3/vmsrcf.cpp
    tads3/vmstack.cpp
    tads3/vmstr.cpp
    tads3/vmstrbuf.cpp
    tads3/vmstrcmp.cpp
    tads3/vmtmpfil.cpp
    tads3/vmtobj.cpp
    tads3/vmtype.cpp
    tads3/vmtypedh.cpp
    tads3/vmtz.cpp
    tads3/vmtzobj.cpp
    tads3/vmundo.cpp
    tads3/vmvec.cpp
# DAP implementation: Conditionally add the debugging files for DAP builds 
    $<$<BOOL:ENABLE_DAP>:tads3/vmdbg.cpp>
    $<IF:$<BOOL:ENABLE_DAP>,tads3/vmbt3_d.cpp,tads3/vmbt3_nd.cpp>
    $<IF:$<BOOL:ENABLE_DAP>,tads3/vmimg_d.cpp,tads3/vmimg_nd.cpp>
    $<IF:$<BOOL:ENABLE_DAP>,tads3/vmini_d.cpp,tads3/vmini_nd.cpp>
    resources.rc
)

# Include directories for the headers. Matching the pro file, we just include ALL header files
target_include_directories(qtads PRIVATE
    htmltads
    src
    tads2
    tads3
)


# Compile time definitions. Some are for tads, others are for qt5
target_compile_definitions(qtads PRIVATE
    QTADS
    QTADS_VERSION="\\\"3.4.0\\\""
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x050A00
    QT_NO_CAST_TO_ASCII
    QT_RESTRICTED_CAST_FROM_ASCII
    QT_USE_QSTRINGBUILDER
    T3_COMPILING_FOR_HTML
    TC_TARGET_T3
    USE_HTML
    VMGLOB_VARS
    VM_FLAT_POOL
    _M_LE_C11
)
## Disable the 
#target_compile_definitions(qtads PRIVATE
#    -D_CRT_SECURE_NO_WARNINGS
#)
# Add the Qt5 libraries
target_link_libraries(qtads PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Widgets
)

if(WIN32)
    target_sources(qtads PUBLIC
        tads2/msdos/ostzw32.c
    )
    target_include_directories(qtads PRIVATE
        tads2/msdos
    )
endif()

# ------------------------------------------------------------------------------
# DAP implementation: Define VM_DEBUGGER and VM_PROFILER for DAP builds 
if(ENABLE_DAP)
    target_compile_definitions(qtads PRIVATE
        VM_DEBUGGER
    )
    # Add DAP Sources here:
    target_sources(qtads PUBLIC
        dap/dap.cpp
    )
endif()
# ------------------------------------------------------------------------------

if(GCC AND (TEST_architecture_arch STREQUAL "i386") AND WIN32)
    target_compile_options(qtads
        -march=i686
        -mtune=generic
    )
endif()

if(GCC AND (TEST_architecture_arch STREQUAL "x86_64") AND WIN32 AND NOT (TEST_architecture_arch STREQUAL "i386"))
    target_compile_options(qtads
        -march=x86-64
        -mtune=generic
    )
endif()

if(MACOS OR WIN32)
    target_compile_definitions(qtads PRIVATE
        OS_NO_TYPES_DEFINED
    )
endif()

if(CLANG OR GCC)
    target_compile_definitions(qtads PRIVATE
        _DEFAULT_SOURCE
        _SVID_SOURCE
    )

    target_compile_options(qtads
        -fno-sized-deallocation
        -fno-strict-aliasing
    )
endif()
#if (CLANG)
#    target_compile_options(qtads Wno-deprecated-declarations)
#endif()

if(DISABLEAUDIO)
    target_compile_definitions(qtads PRIVATE
        NO_AUDIO
    )
endif()

if(NOT DISABLEAUDIO)
    target_sources(qtads PUBLIC
        SDL_audiolib/3rdparty/speex_resampler/resample.c
        SDL_audiolib/src/Decoder.cpp
        SDL_audiolib/src/DecoderFluidsynth.cpp
        SDL_audiolib/src/DecoderMpg123.cpp
        SDL_audiolib/src/DecoderSndfile.cpp
        SDL_audiolib/src/DecoderVorbis.cpp
        SDL_audiolib/src/Processor.cpp
        SDL_audiolib/src/Resampler.cpp
        SDL_audiolib/src/ResamplerSpeex.cpp
        SDL_audiolib/src/Stream.cpp
        SDL_audiolib/src/aulib.cpp
        SDL_audiolib/src/missing/sdl_load_file_rw.c
        SDL_audiolib/src/sampleconv.cpp
        SDL_audiolib/src/stream_p.cpp
        src/rwopsbundle.c src/rwopsbundle.h
    )

    target_include_directories(qtads PRIVATE
        SDL_audiolib
        SDL_audiolib/3rdparty/fmt/include
        SDL_audiolib/3rdparty/speex_resampler
        SDL_audiolib/include
        SDL_audiolib/src
    )

    target_compile_definitions(qtads PRIVATE
        AULIB_STATIC_DEFINE
        FMT_HEADER_ONLY
        OUTSIDE_SPEEX
        RANDOM_PREFIX=SDL_audiolib
        SPX_RESAMPLE_EXPORT=
    )
endif()

if((NOT DISABLEAUDIO) AND (CMAKE_BUILD_TYPE_INIT STREQUAL "Debug"))
    target_compile_definitions(qtads PRIVATE
        AULIB_DEBUG
    )
endif()


if(UNIX)
    target_sources(qtads PUBLIC
        tads2/ostzposix.c
    )
endif()

install(TARGETS qtads
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(ENABLE_DAP)
  # Incluse the unit tests
  include(unittests/unittests.cmake)
endif()

#enable_testing()
#add_executable(Setup unittests/Setup.cpp)
#target_link_libraries(Setup GTest::GTest GTest::Main)
#
#
#if(CLANG OR GCC)
#    target_compile_definitions(Setup PRIVATE
#        _DEFAULT_SOURCE
#        _SVID_SOURCE
#    )
#
#    target_compile_options(Setup
#        -fno-sized-deallocation
#        -fno-strict-aliasing
#    )
#endif()
#
#
#
#gtest_discover_tests(Setup)