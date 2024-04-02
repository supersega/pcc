
# Create the nld interface library
add_library(pcc INTERFACE)

# Set nld compilation features to be propagated to client code.
target_compile_features(pcc INTERFACE cxx_std_20)

# Add the include paths to the nld target
target_include_directories(pcc
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
