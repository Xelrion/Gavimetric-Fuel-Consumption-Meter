# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/david/esp/esp-idf/components/bootloader/subproject"
  "E:/Datos/Proyectos/RTOS/Gavimetric-Fuel-Consumption-Meter/gavimetric_meter/build/bootloader"
  "E:/Datos/Proyectos/RTOS/Gavimetric-Fuel-Consumption-Meter/gavimetric_meter/build/bootloader-prefix"
  "E:/Datos/Proyectos/RTOS/Gavimetric-Fuel-Consumption-Meter/gavimetric_meter/build/bootloader-prefix/tmp"
  "E:/Datos/Proyectos/RTOS/Gavimetric-Fuel-Consumption-Meter/gavimetric_meter/build/bootloader-prefix/src/bootloader-stamp"
  "E:/Datos/Proyectos/RTOS/Gavimetric-Fuel-Consumption-Meter/gavimetric_meter/build/bootloader-prefix/src"
  "E:/Datos/Proyectos/RTOS/Gavimetric-Fuel-Consumption-Meter/gavimetric_meter/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/Datos/Proyectos/RTOS/Gavimetric-Fuel-Consumption-Meter/gavimetric_meter/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "E:/Datos/Proyectos/RTOS/Gavimetric-Fuel-Consumption-Meter/gavimetric_meter/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
