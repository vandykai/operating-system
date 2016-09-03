/* GDTやIDTなどの、 descriptor table 関係 */

#include "bootpack.h"

/**
 * Description: 初始化GDT(全局段号记录表)。
 */
void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) ADR_IDT;
	int i;

	// GDT的初期化
	for (i = 0; i <= LIMIT_GDT / 8; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff,   0x00000000, AR_DATA32_RW);
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);
    // 将GDT(全局段号记录表）的起始地址和有效设定个数放在CPU内被称为GDTR的特殊寄存器中
	load_gdtr(LIMIT_GDT, ADR_GDT);

	// IDT的初期化
	for (i = 0; i <= LIMIT_IDT / 8; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
    // 将IDT(中断记录表）的起始地址和有效设定个数放在CPU内被称为IDTR的特殊寄存器中
	load_idtr(LIMIT_IDT, ADR_IDT);

    // IDT的設定
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, AR_INTGATE32); // 这里的2 * 8表示的是asm_inthandler21属于那一个段，即段号2，乘以8是因为低3位有着别的意思，这里低三位必须是0。
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 * 8, AR_INTGATE32); // 这里的2 * 8表示的是asm_inthandler27属于那一个段，即段号2，乘以8是因为低3位有着别的意思，这里低三位必须是0。
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, AR_INTGATE32); // 这里的2 * 8表示的是asm_inthandler2c属于那一个段，即段号2，乘以8是因为低3位有着别的意思，这里低三位必须是0。

	return;
}

/**Description: 初始化段的描述信息
 * @param sd是段的描述结构体。
 * @param limit的低20位有效，是指定段的上限，也就是段的大小。
 * @param base指的是段的起始地址。
 * @param ar是段的访问权属性，模式如xxxx0000xxxxxxxx，ar的高4位为"扩展访问权限",实际放在limit_high的高4位里。
 * ar低8位含义如下：
 * 0x00:未使用的记录表。
 * 0x92:系统专用，可读写的段，不可执行。
 * 0x9a:系统专用，可执行的段，可读不可写。
 * 0xf2:应用程序用，可读写的段，不可执行。
 * 0xfa:应用程序用，可执行的段，可读不可写。
 * 
 */
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}

