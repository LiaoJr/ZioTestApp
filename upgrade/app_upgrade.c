#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "modbus/modbus.h"


#define MOUNT_POINT           "/mnt/udisk"
#define APP_PATH              "/opt/MasterOfZio/MasterOfZio"
#define BACKUP_PATH           "/opt/MasterOfZio/MasterOfZio_bak"


uint8_t app_upgrade_process(void *p_arg)
{
    int sdx = 0;
    printf("\n>>>>>>app upgrade process...\n");

    /* 1. 检查块设备是否存在（增加重试机制） */
    int retry = 5;
    while (retry-- > 0) {
        if (access("/dev/sda", F_OK) == 0) {
            sdx = 1;
            printf("USB device /dev/sda found\n");
            break;
        }
        if (access("/dev/sdb", F_OK) == 0) {
            sdx = 2;
            printf("USB device /dev/sdb found\n");
            break;
        }
        if (access("/dev/sdc", F_OK) == 0) {
            sdx = 3;
            printf("USB device /dev/sdc found\n");
            break;
        }
        if (access("/dev/sdd", F_OK) == 0) {
            sdx = 4;
            printf("USB device /dev/sdd found\n");
            break;
        }
        sleep(1); // 等待设备识别
    }
    if (retry <= 0) {
        printf("USB device not found\n");
        return 1;
    }

    /* 2. 创建挂载目录 */
    if (mkdir(MOUNT_POINT, 0755) && errno != EEXIST) {
        perror("mkdir failed");
        return 2;
    }

    /* 3. 挂载U盘（建议指定文件系统类型） */
    switch(sdx){
        case 1:
            if (system("mount -t vfat /dev/sda1 " MOUNT_POINT) != 0) {
                printf("Mount failed\n");
                return 3;
            }
            break;
        case 2:
            if (system("mount -t vfat /dev/sdb1 " MOUNT_POINT) != 0) {
                printf("Mount failed\n");
                return 3;
            }
            break;
        case 3:
            if (system("mount -t vfat /dev/sdc1 " MOUNT_POINT) != 0) {
                printf("Mount failed\n");
                return 3;
            }
            break;
        case 4:
            if (system("mount -t vfat /dev/sdd1 " MOUNT_POINT) != 0) {
                printf("Mount failed\n");
                return 3;
            }
            break;
    }

    /* 4. 备份原程序（增加存在性检查） */
    struct stat st;
    if (stat(APP_PATH, &st) == 0) {
        if (rename(APP_PATH, BACKUP_PATH) != 0) {
            perror("Backup failed");
            system("umount " MOUNT_POINT);
            return 4;
        }
    }

    /* 5. 拷贝新程序（使用dd保证二进制完整性） */
    char cmd[256] = {0};
    snprintf(cmd, sizeof(cmd), "dd if=%s/MasterOfZio/MasterOfZio of=%s bs=1M", MOUNT_POINT, APP_PATH);
    if (system(cmd) != 0) {
        printf("Copy failed\n");
        rename(BACKUP_PATH, APP_PATH); // 恢复备份
        system("umount " MOUNT_POINT);
        return 5;
    }

    /* 6. 设置可执行权限 */
    chmod(APP_PATH, 0755);

    /* 7. 卸载U盘 */
    system("umount " MOUNT_POINT);

    printf("\tApp upgrade success!\n");
    return 0;
}

