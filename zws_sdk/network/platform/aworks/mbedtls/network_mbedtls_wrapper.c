﻿/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifdef __cplusplus
extern "C" {
#endif
#ifdef WITH_MBED_TLS
#include <stdbool.h>
#include <string.h>
#include <timer_platform.h>
#include <network_interface.h>

#include "zlg_iot_error.h"
#include "zlg_iot_log.h"
#include "network_interface.h"
#include "network_mbedtls_platform.h"

#include "aworks_platform_common.h"

/* This is the value used for ssl read timeout */
#define IOT_SSL_READ_TIMEOUT 10

/* This defines the value of the debug buffer that gets allocated.
 * The value can be altered based on memory constraints
 */
#ifdef ENABLE_IOT_DEBUG
#define MBEDTLS_DEBUG_BUFFER_SIZE 2048
#endif

/*
 * This is a function to do further verification if needed on the cert received
 */

static int _iot_tls_verify_cert(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags) {
	char buf[1024];
	((void) data);
    (void)depth;

	IOT_DEBUG(ENDL "Verify requested for (Depth %d):" ENDL, depth);
	mbedtls_x509_crt_info(buf, sizeof(buf) - 1, "", crt);
	IOT_DEBUG("%s", buf);

	if((*flags) == 0) {
		IOT_DEBUG("  This certificate has no flags" ENDL);
	} else {
		IOT_DEBUG(buf, sizeof(buf), "  ! ", *flags);
		IOT_DEBUG("%s" ENDL, buf);
	}

	return 0;
}

void _iot_tls_set_connect_params(Network *pNetwork, char *pRootCALocation, char *pDeviceCertLocation,
								 char *pDevicePrivateKeyLocation, char *pDestinationURL,
								 uint16_t destinationPort, uint32_t timeout_ms, bool ServerVerificationFlag) {
	pNetwork->tlsConnectParams.DestinationPort = destinationPort;
	pNetwork->tlsConnectParams.pDestinationURL = pDestinationURL;
	pNetwork->tlsConnectParams.pDeviceCertLocation = pDeviceCertLocation;
	pNetwork->tlsConnectParams.pDevicePrivateKeyLocation = pDevicePrivateKeyLocation;
	pNetwork->tlsConnectParams.pRootCALocation = pRootCALocation;
	pNetwork->tlsConnectParams.timeout_ms = timeout_ms;
	pNetwork->tlsConnectParams.ServerVerificationFlag = ServerVerificationFlag;
}

IoT_Error_t iot_tls_is_connected(Network *pNetwork) {
    (void)pNetwork;
	/* Use this to add implementation which can check for physical layer disconnect */
	return NETWORK_PHYSICAL_LAYER_CONNECTED;
}

IoT_Error_t iot_tls_connect(Network *pNetwork, TLSConnectParams *params) {
	int ret = 0;
	const char *pers = "zlg_iot_tls_wrapper";
	TLSDataParams *tlsDataParams = NULL;
	char portBuffer[6];
	char vrfy_buf[512];

#ifdef ENABLE_IOT_DEBUG
	unsigned char buf[MBEDTLS_DEBUG_BUFFER_SIZE];
#endif

	if(NULL == pNetwork) {
		return NULL_VALUE_ERROR;
	}

	if(NULL != params) {
		_iot_tls_set_connect_params(pNetwork, params->pRootCALocation, params->pDeviceCertLocation,
									params->pDevicePrivateKeyLocation, params->pDestinationURL,
									params->DestinationPort, params->timeout_ms, params->ServerVerificationFlag);
	}

	tlsDataParams = (TLSDataParams*)pNetwork->tlsDataParams;

	mbedtls_net_init(&(tlsDataParams->server_fd));
	mbedtls_ssl_init(&(tlsDataParams->ssl));
	mbedtls_ssl_config_init(&(tlsDataParams->conf));
	mbedtls_ctr_drbg_init(&(tlsDataParams->ctr_drbg));
	mbedtls_x509_crt_init(&(tlsDataParams->cacert));
	mbedtls_x509_crt_init(&(tlsDataParams->clicert));
	mbedtls_pk_init(&(tlsDataParams->pkey));

	IOT_DEBUG(ENDL "  . Seeding the random number generator...");
	mbedtls_entropy_init(&(tlsDataParams->entropy));
	if((ret = mbedtls_ctr_drbg_seed(&(tlsDataParams->ctr_drbg), mbedtls_entropy_func, &(tlsDataParams->entropy),
									(const unsigned char *) pers, strlen(pers))) != 0) {
		IOT_ERROR(" failed" ENDL "  ! mbedtls_ctr_drbg_seed returned -0x%x" ENDL, -ret);
		return NETWORK_MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED;
	}

//#if 0
  if(pNetwork->tlsConnectParams.pRootCALocation){
    IOT_DEBUG("  . Loading the CA root certificate ...");
    // mbedtls_x509_crt_parse_file内部调用fopen、fread读取tlsConnectParams.pRootCALocation文件
    ret = mbedtls_x509_crt_parse_file(&(tlsDataParams->cacert), pNetwork->tlsConnectParams.pRootCALocation);
    if(ret < 0) {
      IOT_ERROR(" failed" ENDL "  !  mbedtls_x509_crt_parse returned -0x%x while parsing root cert" ENDL ENDL, -ret);
      return NETWORK_X509_ROOT_CRT_PARSE_ERROR;
    }
    IOT_DEBUG(" ok (%d skipped)" ENDL, ret);
  }

  if(pNetwork->tlsConnectParams.pDeviceCertLocation){
    IOT_DEBUG("  . Loading the client cert. and key...");
    ret = mbedtls_x509_crt_parse_file(&(tlsDataParams->clicert), pNetwork->tlsConnectParams.pDeviceCertLocation);
    if(ret != 0) {
      IOT_ERROR(" failed" ENDL "  !  mbedtls_x509_crt_parse returned -0x%x while parsing device cert" ENDL ENDL, -ret);
      return NETWORK_X509_DEVICE_CRT_PARSE_ERROR;
    }
  }

  if(pNetwork->tlsConnectParams.pDevicePrivateKeyLocation){
    ret = mbedtls_pk_parse_keyfile(&(tlsDataParams->pkey), pNetwork->tlsConnectParams.pDevicePrivateKeyLocation,);
    if(ret != 0) {
      IOT_ERROR(" failed" ENDL "  !  mbedtls_pk_parse_key returned -0x%x while parsing private key" ENDL ENDL, -ret);
      IOT_DEBUG(" path : %s ", pNetwork->tlsConnectParams.pDevicePrivateKeyLocation);
      return NETWORK_PK_PRIVATE_KEY_PARSE_ERROR;
    }
  }
#endif
	IOT_DEBUG(" ok" ENDL);
	snprintf(portBuffer, 6, "%d", pNetwork->tlsConnectParams.DestinationPort);
	IOT_DEBUG("  . Connecting to %s/%s...", pNetwork->tlsConnectParams.pDestinationURL, portBuffer);
	if((ret = mbedtls_net_connect(&(tlsDataParams->server_fd), pNetwork->tlsConnectParams.pDestinationURL,
								  portBuffer, MBEDTLS_NET_PROTO_TCP)) != 0) {
		IOT_ERROR(" failed" ENDL "  ! mbedtls_net_connect returned -0x%x" ENDL ENDL, -ret);
		switch(ret) {
			case MBEDTLS_ERR_NET_SOCKET_FAILED:
				return NETWORK_ERR_NET_SOCKET_FAILED;
			case MBEDTLS_ERR_NET_UNKNOWN_HOST:
				return NETWORK_ERR_NET_UNKNOWN_HOST;
			case MBEDTLS_ERR_NET_CONNECT_FAILED:
			default:
				return NETWORK_ERR_NET_CONNECT_FAILED;
		};
	}

	ret = mbedtls_net_set_block(&(tlsDataParams->server_fd));
	if(ret != 0) {
		IOT_ERROR(" failed" ENDL "  ! net_set_(non)block() returned -0x%x" ENDL ENDL, -ret);
		return SSL_CONNECTION_ERROR;
	} IOT_DEBUG(" ok" ENDL);

	IOT_DEBUG("  . Setting up the SSL/TLS structure...");
	if((ret = mbedtls_ssl_config_defaults(&(tlsDataParams->conf), MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
										  MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
		IOT_ERROR(" failed" ENDL "  ! mbedtls_ssl_config_defaults returned -0x%x" ENDL ENDL, -ret);
		return SSL_CONNECTION_ERROR;
	}

	mbedtls_ssl_conf_verify(&(tlsDataParams->conf), _iot_tls_verify_cert, NULL);
	if(pNetwork->tlsConnectParams.ServerVerificationFlag == true) {
		mbedtls_ssl_conf_authmode(&(tlsDataParams->conf), MBEDTLS_SSL_VERIFY_REQUIRED);
	} else {
		mbedtls_ssl_conf_authmode(&(tlsDataParams->conf), MBEDTLS_SSL_VERIFY_OPTIONAL);
	}
	mbedtls_ssl_conf_rng(&(tlsDataParams->conf), mbedtls_ctr_drbg_random, &(tlsDataParams->ctr_drbg));

	mbedtls_ssl_conf_ca_chain(&(tlsDataParams->conf), &(tlsDataParams->cacert), NULL);
	if((ret = mbedtls_ssl_conf_own_cert(&(tlsDataParams->conf), &(tlsDataParams->clicert), &(tlsDataParams->pkey))) !=
	   0) {
		IOT_ERROR(" failed" ENDL "  ! mbedtls_ssl_conf_own_cert returned %d" ENDL ENDL, ret);
		return SSL_CONNECTION_ERROR;
	}

	mbedtls_ssl_conf_read_timeout(&(tlsDataParams->conf), pNetwork->tlsConnectParams.timeout_ms);

	if((ret = mbedtls_ssl_setup(&(tlsDataParams->ssl), &(tlsDataParams->conf))) != 0) {
		IOT_ERROR(" failed" ENDL "  ! mbedtls_ssl_setup returned -0x%x" ENDL ENDL, -ret);
		return SSL_CONNECTION_ERROR;
	}
	if((ret = mbedtls_ssl_set_hostname(&(tlsDataParams->ssl), pNetwork->tlsConnectParams.pDestinationURL)) != 0) {
		IOT_ERROR(" failed" ENDL "  ! mbedtls_ssl_set_hostname returned %d" ENDL ENDL, ret);
		return SSL_CONNECTION_ERROR;
	}
	IOT_DEBUG(ENDL ENDL "SSL state connect : %d ", tlsDataParams->ssl.state);
	mbedtls_ssl_set_bio(&(tlsDataParams->ssl), &(tlsDataParams->server_fd), mbedtls_net_send, NULL,
						mbedtls_net_recv_timeout);
	IOT_DEBUG(" ok" ENDL);

	IOT_DEBUG(ENDL ENDL "SSL state connect : %d ", tlsDataParams->ssl.state);
	IOT_DEBUG("  . Performing the SSL/TLS handshake...");
	while((ret = mbedtls_ssl_handshake(&(tlsDataParams->ssl))) != 0) {
		if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
			IOT_ERROR(" failed" ENDL "  ! mbedtls_ssl_handshake returned -0x%x" ENDL, -ret);
			if(ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) {
				IOT_ERROR("    Unable to verify the server's certificate. "
							  "Either it is invalid," ENDL
							  "    or you didn't set ca_file or ca_path "
							  "to an appropriate value." ENDL
							  "    Alternatively, you may want to use "
							  "auth_mode=optional for testing purposes." ENDL);
			}
			return SSL_CONNECTION_ERROR;
		}
	}

	IOT_DEBUG(" ok" ENDL "    [ Protocol is %s ]" ENDL "    [ Ciphersuite is %s ]" ENDL, mbedtls_ssl_get_version(&(tlsDataParams->ssl)),
		  mbedtls_ssl_get_ciphersuite(&(tlsDataParams->ssl)));
	if((ret = mbedtls_ssl_get_record_expansion(&(tlsDataParams->ssl))) >= 0) {
		IOT_DEBUG("    [ Record expansion is %d ]" ENDL, ret);
	} else {
		IOT_DEBUG("    [ Record expansion is unknown (compression) ]" ENDL);
	}

	IOT_DEBUG("  . Verifying peer X.509 certificate...");

	if(pNetwork->tlsConnectParams.ServerVerificationFlag == true) {
		if((tlsDataParams->flags = mbedtls_ssl_get_verify_result(&(tlsDataParams->ssl))) != 0) {
			IOT_ERROR("Server Verification failed" ENDL);
			mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", tlsDataParams->flags);
			IOT_ERROR("%s" ENDL, vrfy_buf);
			ret = SSL_CONNECTION_ERROR;
		} else {
			IOT_DEBUG("Server Verification ok" ENDL);
			ret = SUCCESS;
		}
	} else {
		IOT_DEBUG(" Server Verification skipped" ENDL);
		ret = SUCCESS;
	}

#ifdef ENABLE_IOT_DEBUG
	if (mbedtls_ssl_get_peer_cert(&(tlsDataParams->ssl)) != NULL) {
		IOT_DEBUG("  . Peer certificate information    ..." ENDL);
		mbedtls_x509_crt_info((char *) buf, sizeof(buf) - 1, "      ", mbedtls_ssl_get_peer_cert(&(tlsDataParams->ssl)));
		IOT_DEBUG("%s" ENDL, buf);
	}
#endif

	mbedtls_ssl_conf_read_timeout(&(tlsDataParams->conf), IOT_SSL_READ_TIMEOUT);

	return (IoT_Error_t) ret;
}

IoT_Error_t iot_tls_write(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *written_len) {
	size_t written_so_far;
	bool isErrorFlag = false;
	int frags, ret;
	TLSDataParams *tlsDataParams = (TLSDataParams*)pNetwork->tlsDataParams;

	for(written_so_far = 0, frags = 0;
		written_so_far < len && !has_timer_expired(timer); written_so_far += ret, frags++) {
		while(!has_timer_expired(timer) &&
			  (ret = mbedtls_ssl_write(&(tlsDataParams->ssl), pMsg + written_so_far, len - written_so_far)) <= 0) {
			if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
				IOT_ERROR(" failed" ENDL "  ! mbedtls_ssl_write returned -0x%x" ENDL ENDL, -ret);
				/* All other negative return values indicate connection needs to be reset.
		 		* Will be caught in ping request so ignored here */
				isErrorFlag = true;
				break;
			}
		}
		if(isErrorFlag) {
			break;
		}
	}

	*written_len = written_so_far;

	if(isErrorFlag) {
		return NETWORK_SSL_WRITE_ERROR;
	} else if(has_timer_expired(timer) && written_so_far != len) {
		return NETWORK_SSL_WRITE_TIMEOUT_ERROR;
	}

	return SUCCESS;
}

IoT_Error_t iot_tls_read_any(Network *pNetwork, unsigned char *pMsg, size_t len, size_t *read_len) {
	TLSDataParams *tlsDataParams = (TLSDataParams*)pNetwork->tlsDataParams;
	mbedtls_ssl_context *ssl = &(tlsDataParams->ssl);
	size_t rxLen = 0;
	int ret = 0;

    while(ret <= 0) {
        ret = mbedtls_ssl_read(ssl, pMsg, len);
        if (ret > 0) {
            rxLen += ret;
            pMsg += ret;
            len -= ret;
            break;
        } else if (ret == 0 || (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_TIMEOUT)) {
            return NETWORK_SSL_READ_ERROR;
        } else{
            IOT_DEBUG("mbedtls_ssl_read:%d" ENDL, ret);
            continue;
        }
    }

    *read_len = rxLen;

    return ret >= 0 ? SUCCESS : NETWORK_SSL_READ_ERROR;
}

static IoT_Error_t iot_tls_read_remaining_len(Network *pNetwork, unsigned char *pMsg, size_t len, size_t *read_len) {
	int ret = 0;
	size_t rxLen = 0;
	uint32_t timeout_ms = 3000;

	Timer timer;
	init_timer(&timer);
	countdown_ms(&timer, timeout_ms);
	TLSDataParams *tlsDataParams = (TLSDataParams *)pNetwork->tlsDataParams;
	mbedtls_ssl_context *ssl = &(tlsDataParams->ssl);

	while(len > 0 && !has_timer_expired(&timer)) {
		ret = mbedtls_ssl_read(ssl, pMsg, len);
		if (ret > 0) {
			rxLen += ret;
			pMsg += ret;
			len -= ret;
		} else {   
			if(ret == MBEDTLS_ERR_SSL_TIMEOUT) {
				continue;
			}
			break;
		}
	}

	*read_len = rxLen;

	return len > 0 ? NETWORK_SSL_READ_ERROR : SUCCESS;
}

IoT_Error_t iot_tls_read(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *read_len) {
	TLSDataParams *tlsDataParams = (TLSDataParams*)pNetwork->tlsDataParams;
	mbedtls_ssl_context *ssl = &(tlsDataParams->ssl);
	int timeout = 0;
	size_t rxLen = 0;
	int ret;

	while (len > 0) {
		// This read will timeout after IOT_SSL_READ_TIMEOUT if there's no data to be read
		ret = mbedtls_ssl_read(ssl, pMsg, len);
		if (ret > 0) {
			rxLen += ret;
			pMsg += ret;
			len -= ret;
		} else if (ret == 0 || (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_TIMEOUT)) {
			return NETWORK_SSL_READ_ERROR;
		}

		// Evaluate timeout after the read to make sure read is done at least once
		if (has_timer_expired(timer)) {
			timeout = 1;
			break;
		}
	}

	if(rxLen > 0 && len > 0 && timeout == 1) {
		size_t rem_len = 0;
		iot_tls_read_remaining_len(pNetwork, pMsg, len, &rem_len);
		rxLen += rem_len;
		len -= rem_len;
	}

	if (len == 0) {
		*read_len = rxLen;
		return SUCCESS;
	}

	if (rxLen == 0) {
		return NETWORK_SSL_NOTHING_TO_READ;
	} else {
		return NETWORK_SSL_READ_TIMEOUT_ERROR;
	}
}

IoT_Error_t iot_tls_disconnect(Network *pNetwork) {
	TLSDataParams *tlsDataParams = (TLSDataParams*)pNetwork->tlsDataParams;
	mbedtls_ssl_context *ssl = &(tlsDataParams->ssl);
	int ret = 0;
	do {
		ret = mbedtls_ssl_close_notify(ssl);
	} while(ret == MBEDTLS_ERR_SSL_WANT_WRITE);

	/* All other negative return values indicate connection needs to be reset.
	 * No further action required since this is disconnect call */

	return SUCCESS;
}

IoT_Error_t iot_tls_destroy(Network *pNetwork) {
	TLSDataParams *tlsDataParams = (TLSDataParams*)pNetwork->tlsDataParams;

	mbedtls_net_free(&(tlsDataParams->server_fd));

	mbedtls_x509_crt_free(&(tlsDataParams->clicert));
	mbedtls_x509_crt_free(&(tlsDataParams->cacert));
	mbedtls_pk_free(&(tlsDataParams->pkey));
	mbedtls_ssl_free(&(tlsDataParams->ssl));
	mbedtls_ssl_config_free(&(tlsDataParams->conf));
	mbedtls_ctr_drbg_free(&(tlsDataParams->ctr_drbg));
	mbedtls_entropy_free(&(tlsDataParams->entropy));
    free(tlsDataParams);

	return SUCCESS;
}

static int iot_tls_get_fd(Network *pNetwork)
{
	TLSDataParams *tlsDataParams = (TLSDataParams*)pNetwork->tlsDataParams;

    return tlsDataParams->server_fd.fd;
}


IoT_Error_t iot_tls_init(Network *pNetwork, const char *host, uint16_t port, uint32_t timeout_ms) {
    return iot_tls_init_ex(pNetwork, NULL, NULL, NULL, (char*)host, port, timeout_ms, false);
}

IoT_Error_t iot_tls_init_ex(Network *pNetwork, char *pRootCALocation, char *pDeviceCertLocation,
						 char *pDevicePrivateKeyLocation, char *pDestinationURL,
						 uint16_t destinationPort, uint32_t timeout_ms, bool ServerVerificationFlag) {
	TLSDataParams *tlsDataParams = (TLSDataParams*)calloc(1, sizeof(TLSDataParams));
	_iot_tls_set_connect_params(pNetwork, pRootCALocation, pDeviceCertLocation, pDevicePrivateKeyLocation,
								pDestinationURL, destinationPort, timeout_ms, ServerVerificationFlag);

	pNetwork->getSocket = iot_tls_get_fd;
	pNetwork->connect = iot_tls_connect;
	pNetwork->read = iot_tls_read;
	pNetwork->write = iot_tls_write;
	pNetwork->readAny = iot_tls_read_any;
	pNetwork->disconnect = iot_tls_disconnect;
	pNetwork->isConnected = iot_tls_is_connected;
	pNetwork->destroy = iot_tls_destroy;


	tlsDataParams->flags = 0;
	pNetwork->tlsDataParams = tlsDataParams;

	return SUCCESS;
}

#endif
#ifdef __cplusplus
}
#endif
