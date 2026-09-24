# ---------------------------------------------------------------------------
# The C++ ports of NVIDIA's Learn OpenUSD lessons.
#
# Listed in the curriculum's reading order rather than globbed, so this file
# reads as the same index as README.md and a lesson without a build (06_schemas,
# not yet ported) is absent rather than silently skipped.
#
# Each subdirectory is also a standalone CMake project; see any of their
# CMakeLists.txt.
# ---------------------------------------------------------------------------

add_subdirectory(01_stage)

add_subdirectory(02_prims/define_prim)
add_subdirectory(02_prims/prim_hierarchy)
add_subdirectory(02_prims/traverse_hierarchy)

add_subdirectory(03_properties/attributes)
add_subdirectory(03_properties/relationships)
add_subdirectory(03_properties/material_binding)

add_subdirectory(04_timecodes_timesamples)
add_subdirectory(05_prim_property_paths)
add_subdirectory(07_scopes)
add_subdirectory(08_xform)
add_subdirectory(09_xform_common_api)

add_subdirectory(10_lights/distant_light)
add_subdirectory(10_lights/sphere_light)
