#include "bootpack.h"

void enable_mouse(struct MOUSE_DEC * mdec) {
/* 激活鼠标 */
    /* 鼠标有效 */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    /* 顺利的话，键盘控制送回ACK（0xFA） */
    mdec->phase = 0; /* 等待0xFA的阶段 */
    return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat) {
    if (mdec->phase == 0) {
        /* 等待鼠标的0xFA阶段 */
        if (dat == 0xFA) {
            mdec->phase = 1;
        }
        return 0;
    } else if (mdec->phase == 1) {
        /* 等待鼠标的第一字节阶段 */
        if ((dat & 0xC8) == 0x08) { // 用于判断第一字节对移动有反应的部分（高四位）是否在0 ~ 3的范围内；同时还要判断第一字节对点击有反应的部分（低四位）是否在8 ~ F的范围内;此判断还可以丢弃错位的数据。
            mdec->buf[0] = dat;
            mdec->phase = 2;
        }
        return 0;
    } else if (mdec->phase == 2) {
        /* 等待鼠标的第二字节阶段 */
        mdec->buf[1] = dat;
        mdec->phase = 3;
        return 0;
    } else if (mdec->phase == 3) {
        /* 等待鼠标的第三字节阶段 */
        mdec->buf[2] = dat;
        mdec->phase = 1;

        mdec->btn = mdec->buf[0] & 0x07; // 鼠标键的状态放在buf[0]的低3位  
        mdec->x = mdec->buf[1];
        mdec->y = mdec->buf[2];
        if ((mdec->buf[0] & 0x10) != 0) {
            mdec->x |= 0xFFFFFF00;
        }
        if ((mdec->buf[0] & 0x20) != 0) {
            mdec->y |= 0xFFFFFF00;
        }
        mdec->y = - mdec->y; // 鼠标的y方向与画面符号相反
        return 1;
    }
    return -1; /* 应该不可能到这里来 */
}
