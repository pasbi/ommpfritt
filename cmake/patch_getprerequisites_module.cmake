include(GetPrerequisites RESULT_VARIABLE fn)
execute_process(COMMAND cygpath --unix "${fn}" OUTPUT_VARIABLE fn OUTPUT_STRIP_TRAILING_WHITESPACE)
message("Patching ${fn} ...")

# see https://gitlab.kitware.com/cmake/cmake/-/issues/22381
execute_process(COMMAND sed -i "'s/\"objdump\$\"/\"objdump.exe\$\"/'" "${fn}")
