#ifndef _CPU_H
#define _CPU_H


struct cpu{
    unsigned short PC;
    unsigned char A;
    unsigned char X;
    unsigned char Y;
    unsigned char SP;
    unsigned char P;

};


struct instruction{
    unsigned char opcode;
    unsigned char op0;
    unsigned char op1;
    unsigned char op2;
};

#define STACK_BASE  (0x100)

#define RESET_VEC   (0xFFFA)
#define NMI_VEC     (0xFFFC)
#define IRQ_VEC     (0xFFFE)

/*
0   C   进位标志，如果计算结果产生进位，则置 1
1   Z   零标志，如果结算结果为 0，则置 1
2   I   中断去使能标志，置 1 则可屏蔽掉 IRQ 中断
3   D   十进制模式，未使用
4   B   BRK，后面解释
5   U   未使用，后面解释
6   V   溢出标志，如果结算结果产生了溢出，则置 1
7   N   负标志，如果计算结果为负，则置 1
*/
#define BITMASK_C   (1<<0)
#define BITMASK_Z   (1<<1)
#define BITMASK_I   (1<<2)
#define BITMASK_D   (1<<3)
#define BITMASK_B   (1<<4)
#define BITMASK_U   (1<<5)
#define BITMASK_V   (1<<6)
#define BITMASK_N   (1<<7)

#define SET_C(m)    ((m)=((m)|BITMASK_C))
#define SET_Z(m)    ((m)=((m)|BITMASK_Z))
#define SET_I(m)    ((m)=((m)|BITMASK_I))
#define SET_D(m)    ((m)=((m)|BITMASK_D))
#define SET_B(m)    ((m)=((m)|BITMASK_B))
#define SET_U(m)    ((m)=((m)|BITMASK_U))
#define SET_V(m)    ((m)=((m)|BITMASK_V))
#define SET_N(m)    ((m)=((m)|BITMASK_N))

#define CLR_C(m)    ((m)=((m)&(~BITMASK_C)))
#define CLR_Z(m)    ((m)=((m)&(~BITMASK_Z)))
#define CLR_I(m)    ((m)=((m)&(~BITMASK_I)))
#define CLR_D(m)    ((m)=((m)&(~BITMASK_D)))
#define CLR_B(m)    ((m)=((m)&(~BITMASK_B)))
#define CLR_U(m)    ((m)=((m)&(~BITMASK_U)))
#define CLR_V(m)    ((m)=((m)&(~BITMASK_V)))
#define CLR_N(m)    ((m)=((m)&(~BITMASK_N)))

#define TST_C(m)    ((m)&BITMASK_C)
#define TST_Z(m)    ((m)&BITMASK_Z)
#define TST_I(m)    ((m)&BITMASK_I)
#define TST_D(m)    ((m)&BITMASK_D)
#define TST_B(m)    ((m)&BITMASK_B)
#define TST_U(m)    ((m)&BITMASK_U)
#define TST_V(m)    ((m)&BITMASK_V)
#define TST_N(m)    ((m)&BITMASK_N)


#define CPU_RD      (0)
#define CPU_WRT     (1)
#define IS_ADDR     (0)
#define IS_OP       (1)

extern unsigned char *ram;
extern unsigned char *prg_rom;
extern unsigned char *chr_rom;


extern void cpu_init(void);
extern void cpu_run(void);
#endif
