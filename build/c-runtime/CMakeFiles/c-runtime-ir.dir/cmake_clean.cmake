FILE(REMOVE_RECURSE
  "CMakeFiles/c-runtime-ir"
  "../../lib/c-runtime.bc"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/c-runtime-ir.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
