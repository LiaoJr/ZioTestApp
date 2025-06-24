# Install script for directory: X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/mqttproto")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/aes.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/aesni.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/arc4.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/asn1.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/asn1write.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/base64.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/bignum.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/blowfish.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/bn_mul.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/camellia.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ccm.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/certs.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/check_config.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/cipher.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/cipher_internal.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/cmac.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/compat-1.3.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/config.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ctr_drbg.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/debug.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/des.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/dhm.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ecdh.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ecdsa.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ecjpake.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ecp.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ecp_internal.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/entropy.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/entropy_poll.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/error.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/gcm.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/havege.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/hmac_drbg.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/md.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/md2.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/md4.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/md5.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/md_internal.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/memory_buffer_alloc.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/net.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/net_sockets.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/oid.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/padlock.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/pem.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/pk.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/pk_internal.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/pkcs11.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/pkcs12.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/pkcs5.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/platform.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/platform_time.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ripemd160.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/rsa.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/sha1.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/sha256.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/sha512.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ssl.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ssl_cache.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ssl_ciphersuites.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ssl_cookie.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ssl_internal.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/ssl_ticket.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/threading.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/timing.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/version.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/x509.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/x509_crl.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/x509_crt.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/x509_csr.h"
    "X:/zlgCloud/solar/mqttproto/3rd/mbedtls/include/mbedtls/xtea.h"
    )
endif()

