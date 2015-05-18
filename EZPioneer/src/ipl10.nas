; haribote-ipl
; TAB=4

CYLS	EQU		10				; どこまで読み込むか

		ORG		0x7c00			; 指明程序的装载地址（0x00007c00-0x00007dff是启动区内容的装载地址，IBM的规定）

; 以下的记述用于标准FAT12格式的软盘

		JMP		entry
		DB		0x90
		DB		"HARIBOTE"		; 启动区的名称可以是任意的字符串（8字节）
		DW		512				; 每个扇区（sector）的大小（必须为512字节）
		DB		1				; 簇（cluster）的大小（必须为1个扇区
		DW		1				; FAT的起始位置
		DB		2				; FAT的个数（必须为2）
		DW		224				; 根目录的大小（一般设成224项）
		DW		2880			; 该磁盘的大小（必须是2880扇区）
		DB		0xf0			; 磁盘的种类（必须是0xf0）
		DW		9				; FAT的长度（必须是9扇区）
		DW		18				; 1个磁道（track）有几个扇区（必须是18）
		DW		2				; 磁头数(必须是2）
		DD		0				; 不使用分区，必须是0
		DD		2880			; 重写一次磁盘大小
		DB		0,0,0x29		; 意义不明， 固定
		DD		0xffffffff		; （可能是）卷标号码
		DB		"HARIBOTEOS "	; 磁盘的名称（11字节）
		DB		"FAT12   "		; 磁盘格式名称（8字节）
		RESB	18				; 先空出18字节

; 程序核心

entry:
		MOV		AX,0			; 初始化寄存器
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

; 读磁盘

		MOV		AX,0x0820		; C0-H0-S2(柱面0，磁头0，扇区2的缩写）的存入内存地址（0x08000x081内存地址预留给启动区）
		MOV		ES,AX			; ES为BX段寄存器，0x13中断最后的指定内存的地址为ES*16+BX
								; 汇编默认段寄存器为DS,所以如不指定段寄存器，默认（段寄存器隐藏不写）内存地址要加上DS*16
		MOV		CH,0			; 柱面0
		MOV		DH,0			; 磁头0
		MOV		CL,2			; 扇区2
readloop:
		MOV		SI,0			; 记录失败次数的寄存器
retry:							; 软盘读写容易出错，重试5次都出错就不再尝试，提示给用户出错
		MOV		AH,0x02			; AH=0x02 : 读盘
		MOV		AL,1			; 1个扇区
		MOV		BX,0			; ES:BX为缓冲地址，也就是磁盘读入内存地址
		MOV		DL,0x00			; A驱动器
		INT		0x13			; 调用BIOS，根据AH的值（0x02-读盘，0x03-写盘，0x04-校验，0x0c-寻道）
								; CH、CL、DH、DL分别是柱面号、扇区号、磁头号、驱动器号
								; 一张软盘的大小=80柱面*18扇区*2磁头*512扇区大小=1440KB
								; 启动区位于磁盘C0-H0-S1(柱面0，磁头0，扇区1的缩写）
								; 我们要装载下一个扇区C0-H0-S2(柱面0，磁头0，扇区2的缩写）
		JNC		next			; 没出错的话跳转到next
		ADD		SI,1			; 往SI加1
		CMP		SI,5			; 比较SI与5
		JAE		error			; SI >= 5 时，跳转到error
		MOV		AH,0x00			;（可能是）设为0x00，再调用0x13号中断就是重置驱动器
		MOV		DL,0x00			; A驱动器号
		INT		0x13			; 重置驱动器
		JMP		retry
next:
		MOV		AX,ES			; 把内存地址后移0x200，也可以往BX里加0x200
		ADD		AX,0x0020		; 0x0020 = 512/16转为十六进制
		MOV		ES,AX			; 因为没有ADD ES,0x020指令，所以这里稍微绕个弯
		ADD		CL,1			; 往CL里加1
		CMP		CL,18			; 比较CL与18
		JBE		readloop		; 如果CL <= 18 跳转至readloop
		MOV		CL,1			; 重置为第一个扇区(C0-H0-S18的下一个扇区是C0-H1-S1 )
		ADD		DH,1			; 换为反面磁头
		CMP		DH,2
		JB		readloop		; 如果DH < 2，则跳转到readloop
		MOV		DH,0
		ADD		CH,1			; 柱面号加1
		CMP		CH,CYLS			; CYLS代表欲读入的柱面个数，CH小于CYLS就继续读下一个柱面
		JB		readloop		; 如果CH < CYLS 则跳转到readloop

; 読み終わったのでharibote.sysを実行だ！

		MOV		[0x0ff0],CH		; IPLがどこまで読んだのかをメモ
		JMP		0xc200			;0xc200=0x8000+0x4200(操作系统部分与启动区部分保存在磁盘中，操作系统部分在磁盘中的位置）

error:
		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; 给SI加1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; 显示一个文字
		MOV		BX,15			; 指定字符颜色
		INT		0x10			; 调用显卡BIOS，显示字符
		JMP		putloop
fin:
		HLT						; 让CPU停止，等待指令
		JMP		fin				; 无限循环
msg:
		DB		0x0a, 0x0a		; 换行两次
		DB		"load error"
		DB		0x0a			; 换行
		DB		0

		RESB	0x7dfe-$		; 0x7dfeまでを0x00で埋める命令

		DB		0x55, 0xaa
