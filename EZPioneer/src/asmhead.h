#define ADR_BOOTINFO	0x00000ff0

struct BOOTINFO { /* 0x0ff0-0x0fff */
    char cyls; /* 启动区读硬盘读到何处为止 */
    char leds; /* 启动时键盘的LED的状态 */
    char vmode; /* 显卡模式为多少为彩色 */
    char reserve;
    short scrnx, scrny; /* 画面分辨率 */
    char *vram; //图像缓冲区的开始地址
};
