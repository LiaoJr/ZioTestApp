#include "main.h"
#include "ecat_master.h"
#include "eeprom.h"
#include "log.h"


#define LOG_TAG "MASTER"


ECAT_SIMPLE_START_CONTEXT ctx;
static bool is_reg_handle_success = false;

/**
 * @brief EtherCAT主站请求切换状态
*/
ECAT_RESULT ecat_master_change_state(void *mst_handle, uint8_t state, uint8_t retry_cnt)
{
    ECAT_RESULT ret = ECAT_E_FAIL;

    do
    {
        ret = EcatRequestMasterState((ECAT_HANDLE)mst_handle, state);
        if (ret != ECAT_S_OK)
        {
            LOG_E("EcatRequestMasterState err:%d", ret);
            break;
        }
        LOG_D("state : %d", state);
        while (retry_cnt--)
        {
            ECAT_BYTE query = EcatStateNotSet;
            ret = EcatGetMasterState((ECAT_HANDLE)mst_handle, &query);
            if ((ret == ECAT_S_OK) && (query == state))
            {
                LOG_D("master change state succeess");
                break;
            }
            else
            {
                LOG_D("EcatRequestMasterState err:%d state:%d", ret, query);
                ret = ECAT_E_FAIL;
            }
            LOG_D("retry_cnt : %d", retry_cnt);
            sleep(1);
        }
        if (ret != ECAT_S_OK)
        {
            LOG_D("EcatGetMasterState err:%d", ret);
            break;
        }
    } while (0);
    return ret;
}

/**
 * @brief 获取EtherCAT主站状态
*/
ECAT_RESULT ecat_master_get_state(void *mst_handle, ECAT_BYTE *query)
{        
    return EcatGetMasterState((ECAT_HANDLE)mst_handle, query);
}

/**
 * @brief EtherCAT主站应用初始化
*/
ECAT_RESULT  ecat_master_init(void)
{
    ECAT_RESULT ret = ECAT_E_FAIL;

    memset(&ctx, 0, sizeof(ECAT_SIMPLE_START_CONTEXT));
    ctx.ENI = ECAT_ENI;
    ctx.NIC0 = ECAT_ETH0;
    ctx.MainCycleTimeUs = MAIN_CYCLE_TIME;
    ctx.SubCycleTimeUs = SUB_CYCLE_TIME;
    ctx.AutoRecoveryTimeoutMs = RECOVERY_TIMEOUTMs;
    ctx.RpcServerPort = RPC_SERVER_PORT;
    ret = EcatSimpleStart(&ctx);
    if (ECAT_FAILED(ret)) {
        ecat_master_deinit();
        fprintf(stderr, "EcatSimpleStart() Failed, ret = %d", ret);
        return ret;
    }

    return ret;
}

void ecat_master_wait_test_done(void)
{
    // product_wait_test_done();
    // eeprom_wait_test_done();
}

void ecat_master_set_handle_success(void)
{
    is_reg_handle_success = true;
}

void ecat_master_wait_set_handle_success(void)
{
    while (!is_reg_handle_success)
    {
        sleep(2);
    }
}

/**
 * @brief EtherCAT主站反初始化
*/
void ecat_master_deinit(void)
{
    // is_reg_handle_success = false;
    // eeprom_test_deinit(&ctx);
}
