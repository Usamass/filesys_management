# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/osama-shafiq/esp-idf/components/bootloader/subproject"
  "/home/osama-shafiq/component_exp/build_dir/bootloader"
  "/home/osama-shafiq/component_exp/build_dir/bootloader-prefix"
  "/home/osama-shafiq/component_exp/build_dir/bootloader-prefix/tmp"
  "/home/osama-shafiq/component_exp/build_dir/bootloader-prefix/src/bootloader-stamp"
  "/home/osama-shafiq/component_exp/build_dir/bootloader-prefix/src"
  "/home/osama-shafiq/component_exp/build_dir/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/osama-shafiq/component_exp/build_dir/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/osama-shafiq/component_exp/build_dir/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
