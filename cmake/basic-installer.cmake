# A basic installer setup.
#
# This cmake file introduces two targets
#  ji-lattice-products:      moves all the built assets to a well named directory
#  ji-lattice-installer:   depends on ji-lattice-products, builds an installer
#
# Right now ji-lattice-installer builds just the crudest zip file but this is the target
# on which we will hang the proper installers later

set(JI_LATTICE_PRODUCT_DIR ${CMAKE_BINARY_DIR}/ji-lattice-products)
message(STATUS "Setting up installer for ${PRODUCT_NAME} into ${JI_LATTICE_PRODUCT_DIR}")
file(MAKE_DIRECTORY ${JI_LATTICE_PRODUCT_DIR})

add_custom_target(ji-lattice-products ALL)
add_custom_target(ji-lattice-installer)

function(ji_lattice_package format)
    get_target_property(output_dir ji-lattice-plugin RUNTIME_OUTPUT_DIRECTORY)

    if (TARGET ji-lattice-plugin_${format})
        add_dependencies(ji-lattice-products ji-lattice-plugin_${format})
        add_custom_command(
                TARGET ji-lattice-products
                POST_BUILD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMAND echo "Installing ${output_dir}/${format} to ${JI_LATTICE_PRODUCT_DIR}"
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${output_dir}/${format}/ ${JI_LATTICE_PRODUCT_DIR}/
        )
    endif ()
endfunction()

ji_lattice_package(VST3)
ji_lattice_package(VST)
ji_lattice_package(LV2)
ji_lattice_package(AU)
ji_lattice_package(CLAP)
ji_lattice_package(Standalone)

if (WIN32)
    message(STATUS "Including special windows cleanup installer stage")
    add_custom_command(TARGET ji-lattice-products
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E echo "Cleaning up windows goobits"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${JI_LATTICE_PRODUCT_DIR}/${PRODUCT_NAME}.exp"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${JI_LATTICE_PRODUCT_DIR}/${PRODUCT_NAME}.ilk"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${JI_LATTICE_PRODUCT_DIR}/${PRODUCT_NAME}.lib"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${JI_LATTICE_PRODUCT_DIR}/${PRODUCT_NAME}.pdb"
            )
endif ()

add_dependencies(ji-lattice-installer ji-lattice-products)

add_custom_command(
        TARGET ji-lattice-installer
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND echo "Installing LICENSE and so forth to ${JI_LATTICE_PRODUCT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/resources/LICENSE.distribution ${JI_LATTICE_PRODUCT_DIR}/LICENSE
        # COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Installer/ZipReadme.txt ${JI_LATTICE_PRODUCT_DIR}/Readme.txt
)

find_package(Git)

if (Git_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE VERSION_CHUNK
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else ()
    set(VERSION_CHUNK "unknownhash")
endif ()

string(TIMESTAMP JI_LATTICE_DATE "%Y-%m-%d")
if (WIN32)
    set(JI_LATTICE_ZIP JI-Lattice-${JI_LATTICE_DATE}-${VERSION_CHUNK}-${CMAKE_SYSTEM_NAME}-${BITS}bit.zip)
else ()
    set(JI_LATTICE_ZIP JI-Lattice-${JI_LATTICE_DATE}-${VERSION_CHUNK}-${CMAKE_SYSTEM_NAME}.zip)
endif ()


if (APPLE)
    message(STATUS "Configuring for mac installer.")
    add_custom_command(
            TARGET ji-lattice-installer
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory installer
            COMMAND ${CMAKE_SOURCE_DIR}/libs/sst/sst-plugininfra/scripts/installer_mac/make_installer.sh "${PRODUCT_NAME}" ${CMAKE_BINARY_DIR}/ji-lattice-products ${CMAKE_SOURCE_DIR}/resources/installer_mac ${CMAKE_BINARY_DIR}/installer "${JI_LATTICE_DATE}-${VERSION_CHUNK}"
    )
elseif (WIN32)
    message(STATUS "Configuring for win installer")
    add_custom_command(
            TARGET ji-lattice-installer
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory installer
            COMMAND 7z a -r installer/${JI_LATTICE_ZIP} ${JI_LATTICE_PRODUCT_DIR}/
            COMMAND ${CMAKE_COMMAND} -E echo "ZIP Installer in: installer/${JI_LATTICE_ZIP}")
    find_program(JI_LATTICE_NUGET_EXE nuget.exe PATHS ENV "PATH")
    if(JI_LATTICE_NUGET_EXE)
        message(STATUS "NuGet found at ${JI_LATTICE_NUGET_EXE}")
        add_custom_command(
            TARGET ji-lattice-installer
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${JI_LATTICE_NUGET_EXE} install Tools.InnoSetup -version 6.2.1
            COMMAND Tools.InnoSetup.6.2.1/tools/iscc.exe /O"installer" /DJI_LATTICE_SRC="${CMAKE_SOURCE_DIR}" /DJI_LATTICE_BIN="${CMAKE_BINARY_DIR}" /DMyAppVersion="${JI_LATTICE_DATE}-${VERSION_CHUNK}" "${CMAKE_SOURCE_DIR}/resources/installer_win/jilattice64.iss")
    else()
        message(STATUS "NuGet not found")
    endif()
else ()
    message(STATUS "Basic Installer: Target is installer/${JI_LATTICE_ZIP}")
    add_custom_command(
            TARGET ji-lattice-installer
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory installer
            COMMAND ${CMAKE_COMMAND} -E tar cvf installer/${JI_LATTICE_ZIP} --format=zip ${JI_LATTICE_PRODUCT_DIR}/
            COMMAND ${CMAKE_COMMAND} -E echo "Installer in: installer/${JI_LATTICE_ZIP}")
endif ()
