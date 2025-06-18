#ifndef _SLAVE_H
#define _SLAVE_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "ecat/zecm_zh.h"
#include "ecat/zecm_ex.h"
#include "eeprom.h"
#include "log.h"


ECAT_RESULT get_slave_count(ECAT_HANDLE hMaster, ECAT_WORD *pwCount);

ECAT_RESULT read_eeprom(ECAT_HANDLE hMaster, ECAT_WORD slave_count, ECAT_WORD slave_start, 
                        ECAT_WORD start_addr, ECAT_WORD data_len, ECAT_BYTE **pbuf);

ECAT_RESULT erase_eeprom(ECAT_HANDLE hMaster, ECAT_WORD slave_count, ECAT_WORD slave_start,
                            ECAT_WORD eeprom_size);

ECAT_RESULT write_eeprom(ECAT_HANDLE hMaster, ECAT_WORD slave_count, ECAT_RESULT slave_start,
                         ECAT_WORD start_addr, ECAT_WORD data_len, ECAT_BYTE *pbuf);

ECAT_RESULT read_eeprom_to_file(ECAT_HANDLE hMaster, eeprom_ctx_t *eepromctx_in);

ECAT_RESULT write_eeprom_from_file(ECAT_HANDLE hMaster, eeprom_ctx_t *eepromctx_in);

ECAT_RESULT read_eeprom_for_esi_update(ECAT_HANDLE hMaster, eeprom_ctx_t *eepromctx_in);

ECAT_RESULT slave_mapping_check(ECAT_HANDLE hMaster, eeprom_ctx_t *eepromctx_in);

#endif // _SLAVE_H
