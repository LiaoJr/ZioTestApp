option(USE_STATIC_MBEDTLS_LIBRARY "Build mbed TLS static library." ON)
option(USE_SHARED_MBEDTLS_LIBRARY "Build mbed TLS shared library." OFF)
option(LINK_WITH_PTHREAD "Explicitly link mbed TLS library to pthread." OFF)

set(src_crypto
    aes.c
    aesni.c
    arc4.c
    asn1parse.c
    asn1write.c
    base64.c
    bignum.c
    blowfish.c
    camellia.c
    ccm.c
    cipher.c
    cipher_wrap.c
    cmac.c
    ctr_drbg.c
    des.c
    dhm.c
    ecdh.c
    ecdsa.c
    ecjpake.c
    ecp.c
    ecp_curves.c
    entropy.c
    entropy_poll.c
    error.c
    gcm.c
    havege.c
    hmac_drbg.c
    md.c
    md2.c
    md4.c
    md5.c
    md_wrap.c
    memory_buffer_alloc.c
    oid.c
    padlock.c
    pem.c
    pk.c
    pk_wrap.c
    pkcs12.c
    pkcs5.c
    pkparse.c
    pkwrite.c
    platform.c
    ripemd160.c
    rsa.c
    sha1.c
    sha256.c
    sha512.c
    threading.c
    timing.c
    version.c
    version_features.c
    xtea.c
)

set(src_x509
    certs.c
    pkcs11.c
    x509.c
    x509_create.c
    x509_crl.c
    x509_crt.c
    x509_csr.c
    x509write_crt.c
    x509write_csr.c
)

set(src_tls
    debug.c
    net_sockets.c
    ssl_cache.c
    ssl_ciphersuites.c
    ssl_cli.c
    ssl_cookie.c
    ssl_srv.c
    ssl_ticket.c
    ssl_tls.c
)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations -Wmissing-prototypes")
endif(CMAKE_COMPILER_IS_GNUCC)

if(CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations -Wmissing-prototypes -Wdocumentation -Wno-documentation-deprecated-sync -Wunreachable-code")
endif(CMAKE_COMPILER_IS_CLANG)

if(WIN32)
    set(libs ${libs} ws2_32)
endif(WIN32)

if(USE_PKCS11_HELPER_LIBRARY)
    set(libs ${libs} pkcs11-helper)
endif(USE_PKCS11_HELPER_LIBRARY)

if(ENABLE_ZLIB_SUPPORT)
    set(libs ${libs} ${ZLIB_LIBRARIES})
endif(ENABLE_ZLIB_SUPPORT)

if(LINK_WITH_PTHREAD)
    set(libs ${libs} pthread)
endif()

if (NOT USE_STATIC_MBEDTLS_LIBRARY AND NOT USE_SHARED_MBEDTLS_LIBRARY)
    message(FATAL_ERROR "Need to choose static or shared mbedtls build!")
endif(NOT USE_STATIC_MBEDTLS_LIBRARY AND NOT USE_SHARED_MBEDTLS_LIBRARY)

if(USE_STATIC_MBEDTLS_LIBRARY AND USE_SHARED_MBEDTLS_LIBRARY)
    set(mbedtls_static_target "mbedtls_static")
    set(mbedx509_static_target "mbedx509_static")
    set(mbedcrypto_static_target "mbedcrypto_static")
elseif(USE_STATIC_MBEDTLS_LIBRARY)
    set(mbedtls_static_target "mbedtls")
    set(mbedx509_static_target "mbedx509")
    set(mbedcrypto_static_target "mbedcrypto")
endif()

if(USE_STATIC_MBEDTLS_LIBRARY)
    add_library(${mbedcrypto_static_target} STATIC ${src_crypto})
    set_target_properties(${mbedcrypto_static_target} PROPERTIES OUTPUT_NAME mbedcrypto)
    target_link_libraries(${mbedcrypto_static_target} ${libs})
    set_target_properties(${mbedcrypto_static_target} PROPERTIES FOLDER "3rd/mbedtls")

    add_library(${mbedx509_static_target} STATIC ${src_x509})
    set_target_properties(${mbedx509_static_target} PROPERTIES OUTPUT_NAME mbedx509)
    target_link_libraries(${mbedx509_static_target} ${libs} ${mbedcrypto_static_target})
    set_target_properties(${mbedx509_static_target} PROPERTIES FOLDER "3rd/mbedtls")

    add_library(${mbedtls_static_target} STATIC ${src_tls})
    set_target_properties(${mbedtls_static_target} PROPERTIES OUTPUT_NAME mbedtls)
    target_link_libraries(${mbedtls_static_target} ${libs} ${mbedx509_static_target})
    set_target_properties(${mbedtls_static_target} PROPERTIES FOLDER "3rd/mbedtls")

    install(TARGETS ${mbedtls_static_target} ${mbedx509_static_target} ${mbedcrypto_static_target}
            DESTINATION ${LIB_INSTALL_DIR}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif(USE_STATIC_MBEDTLS_LIBRARY)

if(USE_SHARED_MBEDTLS_LIBRARY)
    add_library(mbedcrypto SHARED ${src_crypto})
    set_target_properties(mbedcrypto PROPERTIES VERSION 2.6.0 SOVERSION 0)
    target_link_libraries(mbedcrypto ${libs})
    set_target_properties(mbedcrypto PROPERTIES FOLDER "3rd/mbedtls")

    add_library(mbedx509 SHARED ${src_x509})
    set_target_properties(mbedx509 PROPERTIES VERSION 2.6.0 SOVERSION 0)
    target_link_libraries(mbedx509 ${libs} mbedcrypto)
    set_target_properties(mbedx509 PROPERTIES FOLDER "3rd/mbedtls")

    add_library(mbedtls SHARED ${src_tls})
    set_target_properties(mbedtls PROPERTIES VERSION 2.6.0 SOVERSION 10)
    target_link_libraries(mbedtls ${libs} mbedx509)
    set_target_properties(mbedtls PROPERTIES FOLDER "3rd/mbedtls")

    install(TARGETS mbedtls mbedx509 mbedcrypto
            DESTINATION ${LIB_INSTALL_DIR}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif(USE_SHARED_MBEDTLS_LIBRARY)

add_custom_target(lib DEPENDS mbedcrypto mbedx509 mbedtls)
set_target_properties(lib PROPERTIES FOLDER "3rd/mbedtls")
if(USE_STATIC_MBEDTLS_LIBRARY AND USE_SHARED_MBEDTLS_LIBRARY)
    add_dependencies(lib mbedcrypto_static mbedx509_static mbedtls_static)
endif()
