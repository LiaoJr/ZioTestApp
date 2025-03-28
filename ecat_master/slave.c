#include "slave.h"
#include "log.h"

ECAT_RESULT get_slave_count(ECAT_HANDLE hMaster, ECAT_WORD *pwCount)
{
    return EcatSSGetSlaveCount(hMaster, pwCount);
}

ECAT_RESULT read_eeprom(ECAT_HANDLE hMaster, ECAT_WORD slave_count, ECAT_WORD slave_start, 
                        ECAT_WORD start_addr, ECAT_WORD data_len, ECAT_BYTE **pbuf)
{
    ECAT_RESULT ret = ECAT_E_FAIL;
    ECAT_BYTE i, j;

    ECAT_WORD *index_buf = (ECAT_WORD *)malloc(slave_count * sizeof(ECAT_WORD));

    for (i = 0, j = slave_start; i < slave_count; i++, j++){
        index_buf[i] = j;
    }
    *pbuf = (ECAT_BYTE *)malloc(slave_count * data_len);
    ret = EcatReadEEPROMByPosIndex(hMaster, slave_count, index_buf, start_addr, data_len, *pbuf);
    free(index_buf);
    return ret;
}

ECAT_RESULT write_eeprom(ECAT_HANDLE hMaster, ECAT_WORD slave_count, ECAT_RESULT slave_start,
                         ECAT_WORD start_addr, ECAT_WORD data_len, ECAT_BYTE *pbuf)
{
    ECAT_RESULT ret = ECAT_E_FAIL;
    ECAT_BYTE i, j;

    ECAT_WORD *index_buf = (ECAT_WORD *)malloc(slave_count * sizeof(ECAT_WORD));

    for (i = 0, j = slave_start; i < slave_count; i++, j++)
        index_buf[i] = j;

    ret = EcatWriteEEPROMByPosIndex(hMaster, slave_count, index_buf,  start_addr, data_len, pbuf);

    free(index_buf);
    return ret;
}

ECAT_RESULT erase_eeprom(ECAT_HANDLE hMaster, ECAT_WORD slave_count, ECAT_WORD slave_start,
                            ECAT_WORD eeprom_size)
{    
    ECAT_RESULT ret = ECAT_E_FAIL;
    ECAT_BYTE i, j;

    ECAT_WORD *index_buf = (ECAT_WORD *)malloc(slave_count * sizeof(ECAT_WORD));
    for (i = 0, j = slave_start; i < slave_count; i++, j++)
        index_buf[i] = j;

    ECAT_BYTE *pbuf = (ECAT_BYTE *)malloc(slave_count * eeprom_size);
    memset(pbuf, 0xff, slave_count * eeprom_size);
    ret = EcatWriteEEPROMByPosIndex(hMaster, slave_count, index_buf, 0, eeprom_size, pbuf);

    free(index_buf);
    free(pbuf);
    return ret;
}

ECAT_RESULT read_eeprom_to_file(ECAT_HANDLE hMaster, eeprom_ctx_t *eepromctx_in)
{
    ECAT_RESULT ret = ECAT_E_FAIL;
    ECAT_BYTE *pbuf = NULL;
    int fd = -1;
    do
    {
        ret = read_eeprom(hMaster, 
                            eepromctx_in->slave_count, 
                            eepromctx_in->slave_start, 
                            eepromctx_in->eeprom_addr, 
                            eepromctx_in->file_size, &pbuf);
        if (ret != ECAT_S_OK){
            break;
        }
        eepromctx_in->UID = *(uint64_t *)(pbuf + eepromctx_in->file_size - 8);
        eepromctx_in->serial_no = *(uint32_t *)(pbuf + 0x1C);
        eepromctx_in->product_id = *(uint32_t *)(pbuf + 0x14);

        fd = open(eepromctx_in->file_name, O_RDWR | O_CREAT, S_IRWXU);
        if (fd < 0)
        {
            ret = ECAT_E_FAIL;
            break;
        }
        ssize_t len = write(fd, pbuf, eepromctx_in->file_size);
        if (len != eepromctx_in->file_size)
            break;

        ret = ECAT_S_OK;
    } while (0);

    if (fd > 0){
        close(fd);
    }
    if (pbuf)
        free(pbuf);
    return ret;
}

ECAT_RESULT write_eeprom_from_file(ECAT_HANDLE hMaster, eeprom_ctx_t *eepromctx_in)
{
    ECAT_RESULT ret = ECAT_E_FAIL;
    int fd = -1;
    ECAT_BYTE *pbuf = malloc(eepromctx_in->slave_count * eepromctx_in->file_size);
    do
    {
        fd = open(eepromctx_in->file_name, O_RDWR, S_IRWXU);
        if (fd < 0)
            break;

        ssize_t len = read(fd, pbuf, eepromctx_in->file_size);
        if (len != eepromctx_in->file_size){
            break;
        }

        /* write Serial No */
        *((uint32_t *)(pbuf + 0x1C)) = eepromctx_in->serial_no;
        /* write UID */
        *((uint64_t *)(pbuf + eepromctx_in->file_size - 8)) = eepromctx_in->UID;
        eepromctx_in->product_id = *(uint32_t *)(pbuf + 0x14);
        ret = write_eeprom(hMaster, 
                            eepromctx_in->slave_count, 
                            eepromctx_in->slave_start, 
                            eepromctx_in->eeprom_addr, 
                            eepromctx_in->file_size, pbuf);
        if (ret != ECAT_S_OK)
            break;

    } while (0);

    if (pbuf)
        free(pbuf);
    if (fd > 0)
        close(fd);
    return ret;
}

ECAT_RESULT read_eeprom_for_esi_update(ECAT_HANDLE hMaster, eeprom_ctx_t *eepromctx_in)
{
    ECAT_RESULT ret = ECAT_E_FAIL;
    ECAT_BYTE *pbuf = NULL;
    int fd = -1;

    do{
        ret = read_eeprom(hMaster, 
                            eepromctx_in->slave_count, 
                            eepromctx_in->slave_start, 
                            eepromctx_in->eeprom_addr, 
                            eepromctx_in->file_size, &pbuf);
        if (ret != ECAT_S_OK){
            break;
        }
        eepromctx_in->UID = *(uint64_t *)(pbuf + eepromctx_in->file_size - 8);
        eepromctx_in->serial_no = *(uint32_t *)(pbuf + 0x1C);
        eepromctx_in->product_id = *(uint32_t *)(pbuf + 0x14);

        get_esi_file_by_pid(eepromctx_in->product_id);

        fd = open(eepromctx_in->file_name, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
        if (fd < 0){
            ret = ECAT_E_FAIL;
            break;
        }
        ssize_t len = write(fd, pbuf, eepromctx_in->file_size);
        if (len != eepromctx_in->file_size)
            break;

        ret = ECAT_S_OK;
    } while (0);

    if (fd > 0){
        close(fd);
    }
    if (pbuf){
        free(pbuf);
    }
    return ret;
}
