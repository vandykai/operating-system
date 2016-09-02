#include "bootpack.h"

void wait_KBC_sendready(void) {
/* 等待键盘控制电路准备完毕 */
    for (;;) {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
    return;
}

/*
* 鼠标控制电路包含在键盘控制电路里，如果键盘控制电路的初始化正常完成，
* 鼠标电路控制器的激活也就完成了。
*/
void init_keyboard(void) {
/* 初始化键盘控制电路 */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
    return;
}