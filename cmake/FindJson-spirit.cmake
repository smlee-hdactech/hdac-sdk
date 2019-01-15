
find_path(LIBJSON_SPIRIT_INCLUDE_DIR json_spirit/json_spirit.h)

find_library(LIBJSON_SPIRIT_LIBRARY NAMES json_spirit)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JSON_SPIRIT DEFAULT_MSG
                                  LIBJSON_SPIRIT_LIBRARY LIBJSON_SPIRIT_INCLUDE_DIR)
mark_as_advanced(LIBJSON_SPIRIT_INCLUDE_DIR LIBJSON_SPIRIT_LIBRARY )
set(JSON_SPIRIT_LIBRARIES ${LIBJSON_SPIRIT_LIBRARY} )
set(JSON_SPIRIT_INCLUDE_DIRS ${LIBJSON_SPIRIT_INCLUDE_DIR} )