FIND_PATH(SDL2_INCLUDE_DIR SDL.h
  HINTS
  $ENV{SDL2}
  PATH_SUFFIXES include/SDL2 include SDL2
  i686-w64-mingw32/include/SDL2
  x86_64-w64-mingw32/include/SDL2
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local/include/SDL2
  /usr/include/SDL2
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)

# Lookup the 64 bit libs on x64
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  FIND_LIBRARY(SDL2_LIBRARY_TEMP SDL2
    HINTS
    $ENV{SDL2}
    PATH_SUFFIXES lib64 lib
    lib/x64
    x86_64-w64-mingw32/lib
    PATHS
    /sw
    /opt/local
    /opt/csw
    /opt
  )
# On 32bit build find the 32bit libs
ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
  FIND_LIBRARY(SDL2_LIBRARY_TEMP SDL2
    HINTS
    $ENV{SDL2}
    PATH_SUFFIXES lib
    lib/x86
    i686-w64-mingw32/lib
    PATHS
    /sw
    /opt/local
    /opt/csw
    /opt
  )
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)

IF(NOT SDL2_BUILDING_LIBRARY)
  IF(NOT ${SDL2_INCLUDE_DIR} MATCHES ".framework")
    # Non-OS X framework versions expect you to also dynamically link to
    # SDL2main. This is mainly for Windows and OS X. Other (Unix) platforms
    # seem to provide SDL2main for compatibility even though they don't
    # necessarily need it.
    # Lookup the 64 bit libs on x64
    IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
      FIND_LIBRARY(SDL2MAIN_LIBRARY
        NAMES SDL2main
        HINTS
        $ENV{SDL2}
        PATH_SUFFIXES lib64 lib
        lib/x64
        x86_64-w64-mingw32/lib
        PATHS
        /sw
        /opt/local
        /opt/csw
        /opt
        )
      # On 32bit build find the 32bit libs
    ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
      FIND_LIBRARY(SDL2MAIN_LIBRARY
        NAMES SDL2main
        HINTS
        $ENV{SDL2}
        PATH_SUFFIXES lib
        lib/x86
        i686-w64-mingw32/lib
        PATHS
        /sw
        /opt/local
        /opt/csw
        /opt
        )
    ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  ENDIF(NOT ${SDL2_INCLUDE_DIR} MATCHES ".framework")
ENDIF(NOT SDL2_BUILDING_LIBRARY)

# SDL2 may require threads on your system.
# The Apple build may not need an explicit flag because one of the
# frameworks may already provide it.
# But for non-OSX systems, I will use the CMake Threads package.
IF(NOT APPLE)
  FIND_PACKAGE(Threads)
ENDIF(NOT APPLE)

# MinGW needs an additional library, mwindows
# It's total link flags should look like -lmingw32 -lSDL2main -lSDL2 -lmwindows
# (Actually on second look, I think it only needs one of the m* libraries.)
IF(MINGW)
  SET(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
ENDIF(MINGW)

SET(SDL2_FOUND "NO")
  IF(SDL2_LIBRARY_TEMP)
    # For SDL2main
    IF(NOT SDL2_BUILDING_LIBRARY)
      IF(SDL2MAIN_LIBRARY)
        SET(SDL2_LIBRARY_TEMP ${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY_TEMP})
      ENDIF(SDL2MAIN_LIBRARY)
    ENDIF(NOT SDL2_BUILDING_LIBRARY)

    # For OS X, SDL2 uses Cocoa as a backend so it must link to Cocoa.
    # CMake doesn't display the -framework Cocoa string in the UI even
    # though it actually is there if I modify a pre-used variable.
    # I think it has something to do with the CACHE STRING.
    # So I use a temporary variable until the end so I can set the
    # "real" variable in one-shot.
    IF(APPLE)
      SET(SDL2_LIBRARY_TEMP ${SDL2_LIBRARY_TEMP} "-framework Cocoa")
    ENDIF(APPLE)

    # For threads, as mentioned Apple doesn't need this.
    # In fact, there seems to be a problem if I used the Threads package
    # and try using this line, so I'm just skipping it entirely for OS X.
    IF(NOT APPLE)
      SET(SDL2_LIBRARY_TEMP ${SDL2_LIBRARY_TEMP} ${CMAKE_THREAD_LIBS_INIT})
    ENDIF(NOT APPLE)

    # For MinGW library
    IF(MINGW)
      SET(SDL2_LIBRARY_TEMP ${MINGW32_LIBRARY} ${SDL2_LIBRARY_TEMP})
    ENDIF(MINGW)

    # Set the final string here so the GUI reflects the final state.
    SET(SDL2_LIBRARY ${SDL2_LIBRARY_TEMP} CACHE STRING "Where the SDL2 Library can be found")
    # Set the temp variable to INTERNAL so it is not seen in the CMake GUI
    SET(SDL2_LIBRARY_TEMP "${SDL2_LIBRARY_TEMP}" CACHE INTERNAL "")

    SET(SDL2_FOUND "YES")
ENDIF(SDL2_LIBRARY_TEMP)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2 REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR)
