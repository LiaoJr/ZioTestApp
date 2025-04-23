PLC通过Modbus TCP连接ZMC600E，目前支持的命令格式：

## EEPROM读写命令[cmd_type=1]
| modbus<br/>寄存器地址 | EEPROM读<br/>[cmd_id=2] | EEPROM写<br/>[cmd_id=2] | EEPROM擦除<br/>[cmd_id=3] | ESI升级<br/>[cmd_id=13] |
| :---: | --- | --- | --- | --- |
| 200 | cmd type:1(16bits) | cmd type:1(16bits) | cmd type:1(16bits) | cmd type:1(16bits) |
| 201 | cmd id:1(16bits) | cmd id:2(16bits) | cmd id:3(16bits) | cmd id:12(16bits) |
| 202 | cmd data(16bits):<br/>file_size(8bit)+<br/>slave_pos(8bits) | cmd data(16bits):<br/>ESI_code(8bit)+<br/>slave_pos(8bits) | cmd data(16bits):<br/>file_size(8bit)+<br/>slave_pos(8bits) | cmd data(16bits):<br/>file_size(8bit)+<br/>slave_pos(8bits) |
| 203 | cmd req(16bits) :<br/>errno(8bits)+<br/>cmd ack(8bits) | cmd req(16bits):<br/>errno(8bits)+<br/>cmd ack(8bits) | cmd req(16bits):<br/>errno(8bits)+<br/>cmd ack(8bits) | cmd req(16bits):<br/>errno(8bits)+<br/>cmd ack(8bits) |


+ file_size specification:  
set bit0 for 256bytes(1)  
set bit1 for 512bytes(2)  
set bit2 for 1024bytes(4)  
set bit3 for 2048bytes(8)  
set bit4 for 4096bytes(16)  
set bit5 for 8192bytes(32)  
set bit6 for 16384bytes(64)  
set bit7 for 32768bytes(128)
+ ESI_Code specification:

```c
typedef enum{
    CMD_DATA_NONE = 0,
    CMD_DATA_ZCPC_E80801 = 1,
    CMD_DATA_ZIOC_E0800AI = 2,
    CMD_DATA_ZIOC_E0800AU = 3,
    CMD_DATA_ZIOC_E0800AU1 = 4,
    CMD_DATA_ZIOC_E0008AU = 5,
    CMD_DATA_ZIOC_E0016DN = 6,
    CMD_DATA_ZIOC_E0016DP = 7,
    CMD_DATA_ZIOC_E1600DN = 8,
    CMD_DATA_ZIOC_E1600DP = 9,
    CMD_DATA_ZIOD_E0808DN = 10,
    CMD_DATA_ZIOD_E0808DP = 11,
    CMD_DATA_ZCPC_E80801_PIO = 12,
    CMD_DATA_ZIOC_E0016DP_PIO = 13,
    CMD_DATA_ZIOC_E1600DP_PIO = 14,
    CMD_DATA_ZIOC_E0008AU_PIO = 15,
    CMD_DATA_ZIOC_E0800AU1_PIO = 16,
    CMD_DATA_ZIOC_E0008AU1 = 17,
    CMD_DATA_MAX_NUM = 18,
}cmd_data_t;
```



## 数字量输出命令[cmd_type=2]
| modbus<br/>寄存器地址 | 电平输出[cmd_id=4] | PWM输出[cmd_id=5] |
| :---: | --- | --- |
| 200 | cmd type:2(16bits) | cmd type:2(16bits) |
| 201 | cmd id:4(16bits) | cmd id:5(16bits) |
| 202 | cmd data(16bits):<br/>IO通道(8bit)+<br/>val值(8bits) | cmd data(16bits):<br/>IO通道(4bit)+<br/>周期ms(6bits)+<br/>有效脉宽ms(6bits) |
| 203 | cmd req(16bits) :<br/>errno(8bits)+<br/>cmd ack(8bits) | cmd req(16bits):<br/>errno(8bits)+<br/>cmd ack(8bits) |




## 数据记录命令[cmd_type=4]
### DO数据记录命令格式[cmd_id=7]
| Modbus字地址(200+x) | 内容 | 描述 |
| --- | --- | --- |
| 0 | cmd_type | 4 |
| 1 | cmd_id | 7 |
| 2 | cmd_data |  |
| 3 | cmd_req/cmd_ack |  |
| 4 | Result_DOs | DO测试结果 |
| 5 | Result_Eeprom_DO | ESI读写测试结果 |
| 6 | Result_Ecat_DO | EtherCAT通信测试 |
| 7 | Result_DOs_Shortcut | DO短路测试结果 |
| 8 | Result_DOs_Carry | DO带载输出测试结果 |
| 9 | Result_LEDs_DO | LED测试结果 |




### DI数据记录命令格式[cmd_id=8]
| Modbus字地址(200+x) | 内容 | 描述 |
| --- | --- | --- |
| 0 | cmd_type | 4 |
| 1 | cmd_id | 8 |
| 2 | cmd_data |  |
| 3 | cmd_req/cmd_ack |  |
| 4 | Result_DIs | DI测试结果 |
| 5 | Result_Eeprom_DI | ESI读写测试结果 |
| 6 | Result_Ecat_DI | EtherCAT通信测试 |
| 7 | Result_DIs_2ms | 4ms周期PWM输入测试结果 |
| 8 | Result_DIs_4ms | 8ms周期PWM输入测试结果 |
| 9 | Result_LEDs_DI | LED测试结果 |




### AO数据记录寄存器格式[cmd_id=9]
| Modbus字地址(200+x) | 内容 | 描述 |
| --- | --- | --- |
| 0 | cmd_type | 4 |
| 1 | cmd_id | 9 |
| 2 | cmd_data |  |
| 3 | cmd_req/cmd_ack |  |
| 4 | Result_AOs | AO测试结果 |
| 5 | Result_ADC_AO | ADC模块通信结果 |
| 6 | Result_Eeprom_AO | ESI读写测试结果 |
| 7 | Result_Ecat_AO | EtherCAT通信测试 |
| 8 | Result_AOs_Cal | AO校准结果 |
| 9 | Result_AOs_Test | AO误差测试结果 |
| 10 | Result_LEDs_AO | LED测试结果 |
| 11 | Reserved | 保留 |
| 12~91 | fAOTestVal[0]~fAOTestVal[39] | 测试点设置值 |
| 92~171 | fAORefVal[0]~fAORefVal[39] | 测试点采样值 |
| 172~251 | fAOMsErr[0]~fAOMsErr[39] | 测量误差 |




### AI数据记录寄存器格式[cmd_id=10]
| Modbus字地址(200+x) | 内容 | 描述 |
| --- | --- | --- |
| 0 | cmd_type | 4 |
| 1 | cmd_id | 10 |
| 2 | cmd_data |  |
| 3 | cmd_req/cmd_ack |  |
| 4 | Result_AIs | AI测试结果 |
| 5 | Result_ADC_AI | ADC模块通信结果 |
| 6 | Result_Eeprom_AI | ESI读写测试结果 |
| 7 | Result_Ecat_AI | EtherCAT通信测试 |
| 8 | Result_AIs_Cal | AI校准结果 |
| 9 | Result_AIs_Test | AI误差测试结果 |
| 10 | Result_LEDs_AI | LED测试结果 |
| 11 | Reserved | 保留 |
| 12~91 | fAITestVal[0]~fAITestVal[39] | 测试点设置值 |
| 92~171 | fAIRefVal[0]~fAIRefVal[39] | 测试点采样值 |
| 172~251 | fAIMsErr[0]~fAIMsErr[39] | 测量误差 |




### DD数据记录寄存器格式[cmd_id=11]
| Modbus字地址(200+x) | 内容 | 描述 |
| --- | --- | --- |
| 0 | cmd_type | 4 |
| 1 | cmd_id | 11 |
| 2 | cmd_data |  |
| 3 | cmd_req/cmd_ack |  |
| 4 | Result_DDs | DI测试结果 |
| 5 | Result_Eeprom_DD | ESI读写测试结果 |
| 6 | Result_Ecat_DD | EtherCAT通信测试 |
| 7 | Result_DDs_2ms | 4ms周期PWM输入测试结果 |
| 8 | Result_DDs_4ms | 8ms周期PWM输入测试结果 |
| 9 | Result_DDs_Shortcut | DO短路测试结果 |
| 10 | Result_DDs_Carry | DO带载输出测试结果 |
| 11 | Result_LEDs_DD | LED测试结果 |




### CP数据记录寄存器格式[cmd_id=6]
| Modbus字地址(200+x) | 内容 | 描述 |
| --- | --- | --- |
| 0 | cmd_type | 4 |
| 1 | cmd_id | 6 |
| 2 | cmd_data |  |
| 3 | cmd_req/cmd_ack |  |
| 4 | Result_CPs | CP测试结果 |
| 5 | Result_Eeprom_CP | ESI读写测试结果 |
| 6 | Result_Ecat_CP | EtherCAT通信测试 |
| 7 | Result_CPs_Vol | CP电压测试测试结果 |
| 8 | Result_LEDs_CP | LED测试结果 |


