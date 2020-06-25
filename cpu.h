#ifndef _CPU_H
#define _CPU_H


struct cpu{
    unsigned short PC;
    unsigned char A;
    unsigned char X;
    unsigned char Y;
    unsigned char SP;
    unsigned char P;
    unsigned long cycle;
};


struct instruction{
    unsigned char opcode;
    unsigned char op0;
    unsigned char op1;
    unsigned char op2;
};

#define STACK_BASE  (0x100)

#define NMI_VEC     (0xFFFA)
#define RESET_VEC   (0xFFFC)
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
#define BIT_C       (0)
#define BIT_Z       (1)
#define BIT_I       (2)
#define BIT_D       (3)
#define BIT_B       (4)
#define BIT_U       (5)
#define BIT_V       (6)
#define BIT_N       (7)

#define BITMASK_C   (1<<BIT_C)
#define BITMASK_Z   (1<<BIT_Z)
#define BITMASK_I   (1<<BIT_I)
#define BITMASK_D   (1<<BIT_D)
#define BITMASK_B   (1<<BIT_B)
#define BITMASK_U   (1<<BIT_U)
#define BITMASK_V   (1<<BIT_V)
#define BITMASK_N   (1<<BIT_N)

#define SET_C(P)    ((P)=((P)|BITMASK_C))
#define SET_Z(P)    ((P)=((P)|BITMASK_Z))
#define SET_I(P)    ((P)=((P)|BITMASK_I))
#define SET_D(P)    ((P)=((P)|BITMASK_D))
#define SET_B(P)    ((P)=((P)|BITMASK_B))
#define SET_U(P)    ((P)=((P)|BITMASK_U))
#define SET_V(P)    ((P)=((P)|BITMASK_V))
#define SET_N(P)    ((P)=((P)|BITMASK_N))

#define CLR_C(P)    ((P)=((P)&(~BITMASK_C)))
#define CLR_Z(P)    ((P)=((P)&(~BITMASK_Z)))
#define CLR_I(P)    ((P)=((P)&(~BITMASK_I)))
#define CLR_D(P)    ((P)=((P)&(~BITMASK_D)))
#define CLR_B(P)    ((P)=((P)&(~BITMASK_B)))
#define CLR_U(P)    ((P)=((P)&(~BITMASK_U)))
#define CLR_V(P)    ((P)=((P)&(~BITMASK_V)))
#define CLR_N(P)    ((P)=((P)&(~BITMASK_N)))

#define TST_C(P)    ((P)&BITMASK_C)
#define TST_Z(P)    ((P)&BITMASK_Z)
#define TST_I(P)    ((P)&BITMASK_I)
#define TST_D(P)    ((P)&BITMASK_D)
#define TST_B(P)    ((P)&BITMASK_B)
#define TST_U(P)    ((P)&BITMASK_U)
#define TST_V(P)    ((P)&BITMASK_V)
#define TST_N(P)    ((P)&BITMASK_N)

#define WRT_C(P,m)    do{ CLR_C(P);  P |= (!!(m))<<BIT_C; }while(0)
#define WRT_Z(P,m)    do{ CLR_Z(P);  P |= (!!(m))<<BIT_Z; }while(0)
#define WRT_I(P,m)    do{ CLR_I(P);  P |= (!!(m))<<BIT_I; }while(0)
#define WRT_D(P,m)    do{ CLR_D(P);  P |= (!!(m))<<BIT_D; }while(0)
#define WRT_B(P,m)    do{ CLR_B(P);  P |= (!!(m))<<BIT_B; }while(0)
#define WRT_U(P,m)    do{ CLR_U(P);  P |= (!!(m))<<BIT_U; }while(0)
#define WRT_V(P,m)    do{ CLR_V(P);  P |= (!!(m))<<BIT_V; }while(0)
#define WRT_N(P,m)    do{ CLR_N(P);  P |= (!!(m))<<BIT_N; }while(0)



#define CPU_RD      (0)
#define CPU_WRT     (1)
#define IS_ADDR     (0)
#define IS_OP       (1)

extern unsigned char *ram;
extern unsigned char *prg_rom;


extern void cpu_init(void);
extern void cpu_reset(void);
extern void cpu_run(void);
extern void cpu_NMI(void);
#endif
