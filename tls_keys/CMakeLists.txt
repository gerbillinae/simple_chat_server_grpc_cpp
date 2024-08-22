# Define output files
set(SERVER_KEY ${CMAKE_BINARY_DIR}/server.key)
set(SERVER_CRT ${CMAKE_BINARY_DIR}/server.crt)
set(SERVER_CSR ${CMAKE_BINARY_DIR}/server.csr)

# Generate the server.key
add_custom_command(
        OUTPUT ${SERVER_KEY}
        COMMAND openssl genpkey -algorithm RSA -out ${SERVER_KEY}
        COMMENT "Generating server.key"
)

# Generate the server.csr using the key
add_custom_command(
        OUTPUT ${SERVER_CSR}
        COMMAND openssl req -new -key ${SERVER_KEY} -out ${SERVER_CSR} -subj "/CN=localhost"
        DEPENDS ${SERVER_KEY}
        COMMENT "Generating server.csr"
)

# Generate the server.crt (self-signed) using the CSR
add_custom_command(
        OUTPUT ${SERVER_CRT}
        COMMAND openssl x509 -req -days 365 -in ${SERVER_CSR} -signkey ${SERVER_KEY} -out ${SERVER_CRT}
        DEPENDS ${SERVER_CSR}
        COMMENT "Generating server.crt"
)

# Create a custom target to group these commands
add_custom_target(tls_keys ALL
        DEPENDS ${SERVER_KEY} ${SERVER_CRT}
)

if(DEFINED DEPLOY_DIR)
   add_custom_command(TARGET tls_keys POST_BUILD
           COMMAND ${CMAKE_COMMAND} -E copy ${SERVER_KEY} ${SERVER_CRT} ${DEPLOY_DIR})
endif()
