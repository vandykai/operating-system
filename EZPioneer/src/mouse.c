#include "bootpack.h"

/*
* �����Ƶ�·�����ڼ��̿��Ƶ�·�������̿��Ƶ�·�ĳ�ʼ��������ɣ�
* ����·�������ļ���Ҳ������ˡ�
*/
void wait_KBC_sendready(void) {
/* �ȴ����̿��Ƶ�·׼����� */
    for (;;) {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
    return;
}

void init_keyboard(void) {
/* ��ʼ�����̿��Ƶ�· */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
    return;
}

void enable_mouse(void) {
/* ������� */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    return; /* ˳���Ļ������̿����䷵���ͻ�ACK��0xFA�� */
}
