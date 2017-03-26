# Copyright (c) 2015 Laurent Noël

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# For MSVC solutions, add a custom command to copy a list of dll files next to the executables after the build
macro(c2ba_copy_dll_post_build EXECUTABLE_FILE DLL_FILES)
    if(MSVC)
        foreach(DLL_FILE ${DLL_FILES})
                add_custom_command(
                    TARGET ${EXECUTABLE_FILE}
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy ${DLL_FILE} ${EXECUTABLE_OUTPUT_PATH}/\$\(Configuration\)
                    COMMENT "copying dll ${DLL_FILE}"
                )
        endforeach()
    endif()
endmacro()