cmake_minimum_required(VERSION 3.28.0)
project(ticket_middle VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Inclua o diretório com os headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Defina os arquivos fonte
set(SOURCES
    main.cpp
    src/ticket_manager.cpp
)

# Defina os arquivos de cabeçalho
set(HEADERS
    include/ticket_manager.hpp
    include/user_session.hpp
)

# Adicione o executável
add_executable(ticket_middle ${SOURCES} ${HEADERS})

# Crie a biblioteca TICKET_MANAGER
add_library(TICKET_MANAGER STATIC
    src/ticket_manager.cpp
    include/ticket_manager.hpp
)

find_package(cpr REQUIRED)
target_link_libraries(TICKET_MANAGER PRIVATE cpr::cpr)



# Configure os diretórios de inclusão para o TICKET_MANAGER
target_include_directories(TICKET_MANAGER PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Configure os diretórios de inclusão para o executável
target_include_directories(ticket_middle PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Vincule as bibliotecas necessárias ao executável
target_link_libraries(ticket_middle PRIVATE TICKET_MANAGER)


# Configurações adicionais (se necessário)
add_compile_options(-Wall -Wextra -Wpedantic)