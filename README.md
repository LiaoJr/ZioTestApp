PLC通过Modbus TCP连接ZMC600E，目前支持的命令格式：
1. EEPROM读写命令
 - EEPROM读：| cmd type:1(16bits) | cmd id:1(bits) | file_size(8bit) slave_pos(8bits) | cmd req(16bits) / errno(8bits) cmd ack |
 - EEPROM写：| cmd type:1(16bits) | cmd id:1(bits) | ESI_code(8bit)  slave_pos(8bits) | cmd req(16bits) / errno(8bits) cmd ack |
 - EEPROM擦：| cmd type:1(16bits) | cmd id:1(bits) | file_size(8bit) slave_pos(8bits) | cmd req(16bits) / errno(8bits) cmd ack |

 * file_size specification:
   set bit0 for 256bytes(1)
   set bit1 for 512bytes(2)
   set bit2 for 1024bytes(4)
   set bit3 for 2048bytes(8)
   set bit4 for 4096bytes(16)
   set bit5 for 8192bytes(32)
   set bit6 for 16384bytes(64)
   set bit7 for 32768bytes(128)

 * ESI_Code specification:
   1. ZCPC-E80801
   2. ZIOC-E0800AI
   3. ZIOC-E0800AU
   4. ZIOC-E0800AU1
   5. ZIOC-E0008AU
