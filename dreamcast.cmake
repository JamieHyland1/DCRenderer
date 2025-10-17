@ -0,0 +1,22 @@
# dreamcast.cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR sh4)

# Just rely on PATH
set(CMAKE_C_COMPILER   sh-elf-gcc)
set(CMAKE_CXX_COMPILER sh-elf-g++)
set(CMAKE_ASM_COMPILER sh-elf-gcc)

# Identify compilers
set(CMAKE_C_COMPILER_ID "GNU")
set(CMAKE_CXX_COMPILER_ID "GNU")
set(CMAKE_ASM_COMPILER_ID "GNU")

# Treat .s as preprocessed assembler
set(CMAKE_ASM_FLAGS "-x assembler-with-cpp" CACHE STRING "" FORCE)

# Prevent try-run
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# Ensure make is found
set(CMAKE_MAKE_PROGRAM "make" CACHE FILEPATH "" FORCE)