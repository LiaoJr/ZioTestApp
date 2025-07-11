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

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <timer_platform.h>
#include <network_interface.h>
//#include <net/if.h> 

#include "zlg_iot_error.h"
#include "zlg_iot_log.h"
#include "network_interface.h"
#include "network_platform.h"

#pragma comment(lib, "Ws2_32.lib")

/* This defines the value of the debug buffer that gets allocated.
 * The value can be altered based on memory constraints
 */

#define h_addr h_addr_list[0]

static int init_winsock() {
  int ret = 0;
  WSADATA wsa_data;
  
  ret = WSAStartup(MAKEWORD(2,2), &wsa_data);
  if(ret != 0) {
    IOT_ERROR("wsa startup failed: %d\n", ret);
    return -1;
  }
  
  return ret;
}

static int socket_connect(const char *host, int port, const char *interface_name) {
  int sock;
  long errcode = 0;
  unsigned long sock_mode = 0;
  int connected = 0;
  struct sockaddr_in s_in;
  memset(&s_in, 0, sizeof(s_in));

  s_in.sin_family = AF_INET;
  struct hostent *h = gethostbyname(host);
  if(h == NULL) {
    IOT_ERROR("gethostbyname failed: %s\n", host);
    return -1;
  }

  memcpy(&s_in.sin_addr.s_addr, h->h_addr, h->h_length);
  s_in.sin_port = htons(port);

  IOT_INFO("connecting...\n");
  if((sock = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    IOT_ERROR("socket error\n");
    return -1;
  }

  /*if (interface_name && strlen(interface_name) > 0) {
    struct ifreq ifr;
    memset(&ifr, 0x00, sizeof(ifr));
    snprintf(ifr.ifr_name, IFNAMSIZ, "%s", interface_name);
    setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr));
  }*/

  sock_mode = 1;
  ioctlsocket(sock, FIONBIO, &sock_mode);
  
  connected = connect(sock, (struct sockaddr *)&s_in, sizeof(struct sockaddr_in));
  if(connected != 0) {
    if((errcode = WSAGetLastError()) != WSAEWOULDBLOCK) {
      IOT_ERROR("connect error : errno=%d\n", errcode);
      closesocket(sock);
      WSACleanup();
      return -1;
    } else {
      struct timeval tm = {3, 0};
      fd_set wset, rset;
      FD_ZERO(&wset);
      FD_ZERO(&rset);
      FD_SET(sock, &wset);
      FD_SET(sock, &rset);
      int res = select(sock + 1, &rset, &wset, NULL, &tm);
      if(res < 0) {
        IOT_ERROR("network error in connect\n");
        closesocket(sock);
        WSACleanup();
        return -1;
      } else if(res == 0) {
        IOT_ERROR("connect time out\n");
        closesocket(sock);
        WSACleanup();
        return -1;
      } else if(1 == res) {
        if(FD_ISSET(sock, &wset)) {
          IOT_INFO("connect succeed.\n");
          sock_mode = 0;
          ioctlsocket(sock, FIONBIO, &sock_mode);
        } else {
          IOT_ERROR("other error when select : errno=%d\n", WSAGetLastError());
          closesocket(sock);
          WSACleanup();
          return -1;
        }
      }
    }
  }

  return sock;
}
/*
 * This is a function to do further verification if needed on the cert received
 */

static void init_param(Network *pNetwork, char *pDestinationURL, uint16_t destinationPort, char *pIFRNName, uint32_t timeout_ms) {
  pNetwork->tlsConnectParams.DestinationPort = destinationPort;
  pNetwork->tlsConnectParams.pDestinationURL = pDestinationURL;
  pNetwork->tlsConnectParams.timeout_ms = timeout_ms;
  pNetwork->tlsConnectParams.pIFRNName = pIFRNName;
}

static IoT_Error_t iot_socket_is_connected(Network *pNetwork) {
  (void)pNetwork;
  /* Use this to add implementation which can check for physical layer disconnect */
  return NETWORK_PHYSICAL_LAYER_CONNECTED;
}

static IoT_Error_t iot_socket_connect(Network *pNetwork, TLSConnectParams *params) {
  uint16_t port = 0;
  const char *host = NULL;
  //size_t timeout_ms = 3000;
  SocketDataParams *s = (SocketDataParams *)pNetwork->tlsDataParams;

  if (NULL != params)
  {
    init_param(pNetwork, params->pDestinationURL, params->DestinationPort, params->pIFRNName, params->timeout_ms);
  }

  host = pNetwork->tlsConnectParams.pDestinationURL;
  port = pNetwork->tlsConnectParams.DestinationPort;
  //timeout_ms = pNetwork->tlsConnectParams.timeout_ms;

  if(init_winsock() == 0) {
    s->sock = socket_connect(host, port, pNetwork->tlsConnectParams.pIFRNName);
  }

  /*if (s->sock >= 0) {
    if (pNetwork->tlsConnectParams.pIFRNName) {
      struct ifreq ifr;
      memset(&ifr, 0x00, sizeof(ifr));
      snprintf(ifr.ifr_name, IFNAMSIZ, pNetwork->tlsConnectParams.pIFRNName);
      setsockopt(s->sock, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr));
    }
  }*/

  return s->sock >= 0 ? SUCCESS : TCP_CONNECTION_ERROR;
}

static IoT_Error_t iot_socket_write(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *written_len) {
  size_t written_so_far;
  bool isErrorFlag = false;
  int frags = 0, ret = 0;
  SocketDataParams *s = (SocketDataParams *)pNetwork->tlsDataParams;

  for(written_so_far = 0, frags = 0; written_so_far < len && !has_timer_expired(timer); written_so_far += ret, frags++) {
    while(!has_timer_expired(timer) && (ret = send(s->sock, pMsg + written_so_far, len - written_so_far, 0)) <= 0) {
      if(ret < 0) {
        isErrorFlag = true;
        IOT_ERROR("socket send failed with error: errno=%d\n", WSAGetLastError());
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
  }
  else if(has_timer_expired(timer) && written_so_far != len) {
    return NETWORK_SSL_WRITE_TIMEOUT_ERROR;
  }

  return SUCCESS;
}

static IoT_Error_t iot_socket_read_any(Network *pNetwork, unsigned char *pMsg, size_t len, size_t *read_len) {
  int ret = 0;
  size_t rxLen = 0;
  long errcode = 0;
  SocketDataParams *s = (SocketDataParams *)pNetwork->tlsDataParams;

  Timer t;
  init_timer_delta(&t, 3, 0);

  while(ret <= 0) {
    ret = recv(s->sock, pMsg, len, 0);
	errcode = WSAGetLastError();
    if(ret > 0) {
      rxLen += ret;
      pMsg += ret;
      len -= ret;
      break;
    } else if (ret == 0 || (errcode != WSAEWOULDBLOCK && errcode != WSAEINTR)) {
      return NETWORK_SSL_READ_ERROR;
    } else if (has_timer_expired(&t)) {
      return NETWORK_SSL_READ_TIMEOUT_ERROR;
    } else {
      IOT_INFO("iot_socket_read:%d\n", ret);
      continue;
    }
  }

  *read_len = ret;

  return ret >= 0 ? SUCCESS : NETWORK_SSL_READ_ERROR;
}

static IoT_Error_t iot_socket_read_remaining_len(Network *pNetwork, unsigned char *pMsg, size_t len, size_t *read_len)
{
  int ret = 0;
  size_t rxLen = 0;
  uint32_t timeout_ms = 3000;

  Timer timer;
  init_timer(&timer);
  countdown_ms(&timer, timeout_ms);
  SocketDataParams *s = (SocketDataParams *)pNetwork->tlsDataParams;

  while(len > 0 && !has_timer_expired(&timer)) {
    ret = recv(s->sock, pMsg, len, 0);
    if (ret > 0) {
      rxLen += ret;
      pMsg += ret;
      len -= ret;
    } else {
      if (WSAGetLastError() == WSAEWOULDBLOCK) {
        continue;
      }
      break;
    }
  }

  *read_len = rxLen;

  return len > 0 ? NETWORK_SSL_READ_ERROR : SUCCESS;
}

static IoT_Error_t iot_socket_read(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *read_len) {
  int ret;
  fd_set rset;
  size_t rxLen = 0;
  int errcode = 0;
  int timeout_sign = 0;
  int nothing_to_read = 0;
  struct timeval timeout;
  SocketDataParams *s = (SocketDataParams *)pNetwork->tlsDataParams;
  
  while (len > 0) {
    FD_ZERO(&rset);
    FD_SET(s->sock, &rset);
    timeout.tv_sec = pNetwork->tlsConnectParams.timeout_ms / 1000;
    timeout.tv_usec = (pNetwork->tlsConnectParams.timeout_ms % 1000) * 1000;
    
    ret = select(0, &rset, NULL, NULL, &timeout);
    if(ret > 0) {
      ret = recv(s->sock, pMsg, len, 0);
      errcode = WSAGetLastError();
      if(ret > 0) {
        rxLen += ret;
        pMsg += ret;
        len -= ret;
      } else if (ret == 0 || (errcode != WSAEWOULDBLOCK && errcode != WSAEINTR)){
        return NETWORK_SSL_READ_ERROR;
      }
    } else if(ret == 0) {
      // 需要继续等待，直接return会造成数据丢失
      /*if(!has_timer_expired(timer)) {
          continue;
      }
      else{
          return NETWORK_SSL_NOTHING_TO_READ;
      }*/      
    } else {
      return NETWORK_SSL_READ_ERROR;
    }
    
    if(has_timer_expired(timer)) {
      timeout_sign = 1;
      break;
    }
  }
  
  if(rxLen > 0 && len > 0 && timeout_sign == 1) {
    size_t rem_len = 0;
    iot_socket_read_remaining_len(pNetwork, pMsg, len, &rem_len);
    rxLen += rem_len;
    len -= rem_len;
  }
  
  *read_len = rxLen;
  if(len == 0) {
    return SUCCESS;
  }
  
  return NETWORK_SSL_READ_TIMEOUT_ERROR;
}

static IoT_Error_t iot_socket_disconnect(Network *pNetwork) {
  SocketDataParams *s = (SocketDataParams *)pNetwork->tlsDataParams;

  if(s->sock >= 0) {
    closesocket(s->sock);
    WSACleanup();
    s->sock = -1;
  }

  return SUCCESS;
}

static IoT_Error_t iot_socket_destroy(Network *pNetwork) {
  SocketDataParams *s = (SocketDataParams *)pNetwork->tlsDataParams;

  if(s->sock >= 0) {
    closesocket(s->sock);
    WSACleanup();
    s->sock = -1;
  }
  free(s);
  
  return SUCCESS;
}

static int iot_socket_get_fd(Network *pNetwork) {
  SocketDataParams *s = (SocketDataParams *)pNetwork->tlsDataParams;

  return s->sock;
}

IoT_Error_t iot_socket_init(Network *pNetwork, const char *host, uint16_t port, const char *interface_name, uint32_t timeout_ms) {
  SocketDataParams *tlsDataParams = (SocketDataParams *)calloc(1, sizeof(SocketDataParams));
  init_param(pNetwork, (char *)host, port, interface_name, timeout_ms);

  pNetwork->getSocket = iot_socket_get_fd;
  pNetwork->connect = iot_socket_connect;
  pNetwork->read = iot_socket_read;
  pNetwork->readAny = iot_socket_read_any;
  pNetwork->write = iot_socket_write;
  pNetwork->disconnect = iot_socket_disconnect;
  pNetwork->isConnected = iot_socket_is_connected;
  pNetwork->destroy = iot_socket_destroy;

  tlsDataParams->sock = -1;
  pNetwork->tlsDataParams = tlsDataParams;

  return SUCCESS;
}

#ifdef __cplusplus
}
#endif
