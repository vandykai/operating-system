#define KEYCMD_SENDTO_MOUSE     0xd4
#define MOUSECMD_ENABLE         0xf4

struct MOUSE_DEC {
    unsigned char buf[3], phase;
    int x, y, btn;
};

void wait_KBC_sendready(void);
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
