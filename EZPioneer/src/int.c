/* 割り込み関係 */

#include "bootpack.h"

/**
 * 初始化PIC，PIC有一个IMR寄存器，四个ICW寄存器，ICW1, ICW3, ICW4的值不能随意设定，
 * 否者老的计算机可能被烧坏保险，新的计算机会无视无效设定。
 */
void init_pic(void) {
//PIC的初始化
    io_out8(PIC0_IMR, 0xFF); // 禁止所有中断
    io_out8(PIC1_IMR, 0xFF); // 禁止所有中断

    io_out8(PIC0_ICW1, 0x11); // 边沿触发模式（edge trigger mode）
    io_out8(PIC0_ICW2, 0x20); // IRQ0-7由INT20-27接收，INT 0x00~0x2F不能用于IRQ，之所以不能用，是因为应用程序想要对操作系统干坏事的时候，CPU内部会自动产生INT 0x00~0x1F。
    io_out8(PIC0_ICW3, 1 << 2); // PIC1由IRQ2连接
    io_out8(PIC0_ICW4, 0x01); // 无缓冲区模式

    io_out8(PIC1_ICW1, 0x11); // 边沿触发模式（edge trigger mode）
    io_out8(PIC1_ICW2, 0x28); // IRQ8-15由INT28-2F接收
    io_out8(PIC1_ICW3, 2); // PIC1由IRQ2连接
    io_out8(PIC1_ICW4, 0x01); // 无缓冲区模式

    io_out8(PIC0_IMR, 0xFB); // 11111011 IRQ2即PIC1以外全部禁止
    io_out8(PIC1_IMR, 0xFF); // 11111111 禁止所有中断

    return;
}

struct FIFO8 keyfifo;

/**
 * 来自PS/2键盘的中断
 */
void inthandler21(int *esp) {
    unsigned char data;
    io_out8(PIC0_OCW2, 0x61); // 0x61 = 0x60 + IRQ号码（这里为1），用来通知PIC0继续监视IRQ1中断，等于鼓励信号。
    data = io_in8(PORT_KEYDAT);

    fifo8_put(&keyfifo, data);
    return;
}

struct FIFO8 mousefifo;

/**
 * 来自PS/2鼠标的中断
 */
void inthandler2c(int *esp) {
    unsigned char data;
    io_out8(PIC1_OCW2, 0x64); // 0x64 = 0x60 + IRQ号码（这里为4），用来通知PIC1继续监视IRQ12(从PIC相当于IRQ -08 ~ IRQ -15 4 + 8 = 12)中断，等于鼓励信号。
    io_out8(PIC0_OCW2, 0x62); // 0x62 = 0x60 + IRQ号码（这里为2），用来通知PIC0继续监视IRQ2中断，等于鼓励信号。
    data = io_in8(PORT_KEYDAT);
    fifo8_put(&mousefifo, data);
    return;
}

void inthandler27(int *esp)
/* PIC0からの不完全割り込み対策 */
/* Athlon64X2機などではチップセットの都合によりPICの初期化時にこの割り込みが1度だけおこる */
/* この割り込み処理関数は、その割り込みに対して何もしないでやり過ごす */
/* なぜ何もしなくていいの？
    →  この割り込みはPIC初期化時の電気的なノイズによって発生したものなので、
        まじめに何か処理してやる必要がない。                                    */
{
    io_out8(PIC0_OCW2, 0x67); /* IRQ-07受付完了をPICに通知(7-1参照) */
    return;
}
