# 1. Buscar recursivamente todos los archivos .cpp y .c
file(GLOB_RECURSE app_sources "*.cpp" "*.c")

# 2. Registrar el componente usando la lista generada
idf_component_register(SRCS ${app_sources}
                       INCLUDE_DIRS ".")