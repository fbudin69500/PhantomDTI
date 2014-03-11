cmake_minimum_required(VERSION 2.6)

project( DTIPhantom )

find_package( SlicerExecutionModel REQUIRED )
include(${SlicerExecutionModel_USE_FILE})
find_package( ITK REQUIRED )
include( ${USE_ITK_FILE} )


SEMMacroBuildCLI(
    NAME DTIPhantom
    EXECUTABLE_ONLY
    TARGET_LIBRARIES ${ITK_LIBRARIES}
)

