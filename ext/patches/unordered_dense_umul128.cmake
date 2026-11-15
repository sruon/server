# MSVC 19.52 miscompiles _umul128 when mum() is inlined into a coroutine, so the high half comes from __umulh.
# Runs as the unordered_dense patch step, from its source directory.
set(header "include/ankerl/unordered_dense.h")
set(original "    *a = _umul128(*a, *b, b);")
set(replacement "    auto const high = __umulh(*a, *b);\n    *a = *a * *b;\n    *b = high;")

file(READ "${header}" content)

string(FIND "${content}" "${replacement}" patched)
if(NOT patched EQUAL -1)
    return()
endif()

string(FIND "${content}" "${original}" found)
if(found EQUAL -1)
    message(FATAL_ERROR "unordered_dense: the _umul128 line moved, update ext/patches/unordered_dense_umul128.cmake")
endif()

string(REPLACE "${original}" "${replacement}" content "${content}")
file(WRITE "${header}" "${content}")
