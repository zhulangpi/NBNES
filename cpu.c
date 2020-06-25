#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "ppu.h"

#define DEBUG

#define cpu_read16(addr)        ( cpu_addr(addr, 0, CPU_RD) |(cpu_addr(addr+1, 0, CPU_RD)<<8) )

#define diff_page(m,n)          (((m)&0xFF00)!=((n)&0xFF00))


unsigned char *ram;
unsigned char *prg_rom;
struct cpu *cpu;


void exec_one_inst(struct cpu *c);




//addr cpu bus data
unsigned char cpu_addr(unsigned short addr, unsigned char in, int rw)
{
    unsigned char data;

    if( addr<0x2000 ){  //RAM
        addr &= 0x7FF;
        if(rw==CPU_WRT){
            ram[addr] = in;
        }
       // if(addr==0)
        //printf("%s: %x %x\n", (dir==CPU_RD)?"read":"write", addr, ram[addr]);
        return ram[addr];

    }else if( addr<0x4000 ){    //IO Register
        return ppu_reg_rw(addr, data, rw);

    }else if( addr<0x4020 ){    //IO Register

    }else if( addr<0x6000 ){    //Expansion ROM
        addr -= 0x4020;

    }else if( addr<0x8000 ){    //SRAM
        addr -= 0x6000;

    }else if( addr<0x10000 ){   //PRG-ROM
        addr -= 0x8000;

        if(addr>=0x4000){
            addr-=0x4000;
        }

        if(rw==CPU_WRT){
            prg_rom[addr] = in;
        }
        return prg_rom[addr];
    }

    return 0;
}


void cpu_reset(void)
{
    SET_I(cpu->P);
    cpu->SP -= 3;
    cpu->PC = cpu_read16(RESET_VEC);
    cpu->cycle = 7;
}


void cpu_init(void)
{
    ram = (unsigned char*)malloc(0x800);
    memset(ram, 0, 0x800);
    cpu = (struct cpu*)malloc(sizeof(struct cpu));
    cpu->PC = 0;
    cpu->P = 0x24;
    cpu->SP = 0;
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->cycle = 0;
    printf("NMI_VEC:%#x\n", cpu_read16(NMI_VEC));
    printf("RESET_VEC:%#x\n", cpu_read16(RESET_VEC));
    printf("IRQ_VEC:%#x\n", cpu_read16(IRQ_VEC));
    cpu_reset();
    cpu->PC = 0xC000;
}


void push(struct cpu *c, unsigned char m)
{
    cpu_addr(c->SP + STACK_BASE, m, CPU_WRT);
    c->SP--;
}

unsigned char pop(struct cpu *c) 
{
    c->SP++;
    return cpu_addr(c->SP + STACK_BASE, 0, CPU_RD);
}


void print_cpu(struct cpu* c)
{
    static int i= 0;
    printf("%4d: A:%2x X:%2x Y:%2x P:%2x SP:%2x ",++i, c->A, c->X, c->Y, c->P, c->SP );
}

void cpu_run(void)
{
#ifdef DEBUG
    print_cpu(cpu);
#endif
    exec_one_inst(cpu);
}


void cpu_NMI(void)
{
    push(cpu, (unsigned char)(cpu->PC >> 8) );
    push(cpu, (unsigned char) cpu->PC);
    CLR_U(cpu->P);
    SET_I(cpu->P);
    push(cpu, cpu->P);
    //push(cpu, cpu->P|BITMASK_U);
    cpu->PC = cpu_read16(NMI_VEC);
}


//instruction exection
//if op may be A, return op;
//else return 0;
unsigned char exec_ORA(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    c->A |= op;
    WRT_Z(c->P, 0==c->A);
    WRT_N(c->P, 0x80 & c->A);
    return 0;
}

unsigned char exec_ASL(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    WRT_C(c->P, op & 0x80);
    op <<= 1;
    WRT_N(c->P, op & 0x80);
    if(flag==IS_ADDR){
        return cpu_addr(addr, op, CPU_WRT);
    }else{
        return op;
    }
}

unsigned char exec_BIT(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    WRT_Z(c->P, !(op & c->A));
    WRT_V(c->P, op & 0x40);
    WRT_N(c->P, op & 0x80);
    return 0;
}

unsigned char exec_AND(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    c->A &= op;
    WRT_Z(c->P, c->A==0);
    WRT_N(c->P, c->A & 0x80);
    return 0;
}

unsigned char exec_EOR(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    c->A ^= op;
    if(c->A==0){
        SET_Z(c->P);
    }else if(c->A & 0x80){
        SET_N(c->P);
    }
    return 0;
}

unsigned char exec_LSR(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;

    WRT_C(c->P, op & 0x01);
    op >>= 1;
    op &= 0x7F;
    WRT_Z(c->P, op == 0);
    CLR_N(c->P);
    if(flag==IS_ADDR){
        return cpu_addr(addr, op, CPU_WRT);
    }else{
        return op;
    }
}

unsigned char exec_ROL(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;

    WRT_C(c->P, op & 0x80);
    op <<= 1;
    if(TST_C(c->P)){
        op |= 0x01;
    }
    WRT_N(c->P, op & 0x80);
    if(flag==IS_ADDR){
        return cpu_addr(addr, op, CPU_WRT);
    }else{
        return op;
    }
}

unsigned char exec_ROR(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char old_P = c->P;
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;

    WRT_C(c->P, op & 0x01);
    op >>= 1;
    if(TST_C(old_P)){
        op |= 0x80;
        SET_N(c->P);
    }else{
        CLR_N(c->P);
    }
    if(flag==IS_ADDR){
        return cpu_addr(addr, op, CPU_WRT);
    }else{
        return op;
    }
}

unsigned char exec_ADC(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    unsigned char C = TST_C(c->P)?1:0;
    unsigned int  sum = c->A + op + C;
    unsigned char sum8 = c->A + op + C;

    WRT_C(c->P, sum > 0xFF);    //overflow
    WRT_V(c->P, (~(c->A ^ op)) & (c->A ^ sum8) & 0x80);

    c->A = c->A + op + C;

    WRT_N(c->P, c->A & 0x80);
    WRT_Z(c->P, c->A==0);
    return 0;
}


unsigned char exec_SBC(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    unsigned char C = TST_C(c->P)?0:1;
    unsigned int sum = c->A - op - C;
    unsigned char sum8 = c->A - op - C;

    WRT_C(c->P, sum <= 0xFF);    //overflow
    WRT_V(c->P, (c->A ^ op) & (c->A ^ sum8) & 0x80);

    c->A = c->A - op - C;
    WRT_N(c->P, c->A & 0x80);
    WRT_Z(c->P, c->A==0);
    return 0;
}

unsigned char exec_LDA(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    c->A = op;
    WRT_N(c->P, c->A & 0x80);
    WRT_Z(c->P, c->A==0);
    return 0;
}

unsigned char exec_LDX(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    c->X = op;
    WRT_N(c->P, c->X & 0x80);
    WRT_Z(c->P, c->X==0);
    return 0;
}

unsigned char exec_LDY(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    c->Y = op;
    WRT_N(c->P, c->Y & 0x80);
    WRT_Z(c->P, c->Y==0);
    return 0;
}

unsigned char exec_CMP(unsigned short addr, struct cpu *c, int flag)
{
    int s_data;
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    s_data = c->A - op;
    WRT_C(c->P, s_data>=0);
    WRT_Z(c->P, s_data==0);
    WRT_N(c->P, s_data & 0x80);
    return 0;
}

unsigned char exec_CPX(unsigned short addr, struct cpu *c, int flag)
{
    int s_data;
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    s_data = c->X - op;
    WRT_C(c->P, s_data>=0);
    WRT_Z(c->P, s_data==0);
    WRT_N(c->P, s_data & 0x80);
    return 0;
}

unsigned char exec_CPY(unsigned short addr, struct cpu *c, int flag)
{
    int s_data;
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    s_data = c->Y - op;
    WRT_C(c->P, s_data>=0);
    WRT_Z(c->P, s_data==0);
    WRT_N(c->P, s_data & 0x80);
    return 0;
}


unsigned char exec_STA(unsigned short addr, struct cpu *c, int flag)
{
    (void)flag;
    cpu_addr(addr, c->A, CPU_WRT);
    return 0;
}

unsigned char exec_STX(unsigned short addr, struct cpu *c, int flag)
{
    (void)flag;
    cpu_addr(addr, c->X, CPU_WRT);
    return 0;
}

unsigned char exec_STY(unsigned short addr, struct cpu *c, int flag)
{
    (void)flag;
    cpu_addr(addr, c->Y, CPU_WRT);
    return 0;
}

unsigned char exec_DEC(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char data = cpu_addr(addr, 0 ,CPU_RD);
    (void)flag;
    data--;
    cpu_addr(addr, data, CPU_WRT);
    WRT_N(c->P, data & 0x80);
    WRT_Z(c->P, data==0);
    return 0;
}

unsigned char exec_INC(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char data = cpu_addr(addr, 0 ,CPU_RD);
    (void)flag;
    data++;
    cpu_addr(addr, data, CPU_WRT);
    WRT_N(c->P, data & 0x80);
    WRT_Z(c->P, data==0);
    return 0;
}

//unoffical instruction
unsigned char exec_NOP(unsigned short addr, struct cpu *c, int flag)
{
    return 0;
}

unsigned char exec_LAX(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    c->A = op;
    WRT_N(c->P, c->A & 0x80);
    WRT_Z(c->P, c->A==0);
    c->X = c->A;
    WRT_N(c->P, c->X & 0x80);
    WRT_Z(c->P, c->X==0);
    return 0;

}

unsigned char exec_SAX(unsigned short addr, struct cpu *c, int flag)
{
    cpu_addr(addr, c->A&c->X, CPU_WRT);
    return 0;
}

unsigned char exec_DCP(unsigned short addr, struct cpu *c, int flag)
{
    int s_data;
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    op--;
    cpu_addr(addr, op, CPU_WRT);
    s_data = c->A - op;
    WRT_C(c->P, s_data>=0);
    WRT_Z(c->P, s_data==0);
    WRT_N(c->P, s_data & 0x80);
    return 0;
}


unsigned char exec_ISC(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    unsigned char C = TST_C(c->P)?0:1;
    unsigned int sum;
    unsigned char sum8;
    op++;
    cpu_addr(addr, op, CPU_WRT);
    sum = c->A - op - C;
    sum8 = c->A - op - C;

    WRT_C(c->P, sum <= 0xFF);
    WRT_V(c->P, (c->A ^ op) & (c->A ^ sum8) & 0x80);

    c->A = c->A - op - C;
    WRT_N(c->P, c->A & 0x80);
    WRT_Z(c->P, c->A==0);
    return 0;
}


unsigned char exec_SLO(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;

    WRT_C(c->P, op & 0x80);
    op <<= 1;
    c->A |= op;

    WRT_N(c->P, c->A & 0x80);
    WRT_Z(c->P, c->A==0);
    if(flag==IS_ADDR){
        return cpu_addr(addr, op, CPU_WRT);
    }else{
        return op;
    }
}

//zlp may be a bug because of the nestest.log
unsigned char exec_RLA(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char op = (flag==IS_ADDR)?cpu_addr(addr, 0 ,CPU_RD):addr;
    unsigned char old_op = op;
    op <<= 1;
    if(TST_C(c->P)){
        op |= 0x01;
    }
    c->A &= op;

    WRT_N(c->P, c->A & 0x80);
    WRT_Z(c->P, c->A==0);
    WRT_C(c->P, old_op & 0x80);
    if(flag==IS_ADDR){
        return cpu_addr(addr, op, CPU_WRT);
    }else{
        return op;
    }
}

unsigned char exec_SRE(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char ret;
    ret = exec_LSR(addr,c,flag);
    exec_EOR(addr,c,flag);
    return ret;
}

unsigned char exec_RRA(unsigned short addr, struct cpu *c, int flag)
{
    unsigned char ret;
    ret = exec_ROR(addr,c,flag);
    exec_ADC(addr,c,flag);
    return ret;
}




//address mode
void inst_imm(struct instruction *inst, struct cpu *c, unsigned char (*f)(unsigned short, struct cpu *, int) )
{
    unsigned op = inst->op0;
    f(op, c, IS_OP);
    c->PC+=2;
}

void inst_A(struct instruction *inst, struct cpu *c, unsigned char (*f)(unsigned short, struct cpu *, int) )
{
    unsigned char op = c->A;
    c->A = f(op, c, IS_OP);
    WRT_N(c->P, c->A & 0x80);
    WRT_Z(c->P, c->A==0);
    c->PC++;
}

void inst_zp(struct instruction *inst, struct cpu *c, unsigned char (*f)(unsigned short, struct cpu *, int) )
{
    unsigned short addr;
    addr = inst->op0;
    f(addr, c, IS_ADDR);
    c->PC+=2;
}

void inst_zpx(struct instruction *inst, struct cpu *c, unsigned char (*f)(unsigned short, struct cpu *, int) )
{
    unsigned short addr;
    addr = inst->op0 + c->X;
    addr &= 0xFF;
    f(addr, c, IS_ADDR);
    c->PC+=2;
}

void inst_zpy(struct instruction *inst, struct cpu *c, unsigned char (*f)(unsigned short, struct cpu *, int) )
{
    unsigned short addr;
    addr = inst->op0 + c->Y;
    addr &= 0xFF;
    f(addr, c, IS_ADDR);
    c->PC+=2;
}

void inst_abs(struct instruction *inst, struct cpu *c, unsigned char (*f)(unsigned short, struct cpu *, int) )
{
    unsigned short addr;
    addr = (inst->op1 << 8) | inst->op0;
    f(addr, c, IS_ADDR);
    c->PC+=3;
}

int inst_abx(struct instruction *inst, struct cpu *c, unsigned char (*f)(unsigned short, struct cpu *, int) )
{
    unsigned short addr, old_addr;
    old_addr = ((inst->op1 << 8) | inst->op0);
    addr = old_addr + c->X;
    f(addr, c, IS_ADDR);
    c->PC+=3;
    return diff_page(old_addr, addr);
}

int inst_aby(struct instruction *inst, struct cpu *c, unsigned char (*f)(unsigned short, struct cpu *, int) )
{
    unsigned short addr, old_addr;
    old_addr =  ((inst->op1 << 8) | inst->op0);
    addr =  old_addr + c->Y;
    f(addr, c, IS_ADDR);
    c->PC+=3;
    return diff_page(old_addr, addr);
}

void inst_izx(struct instruction *inst, struct cpu *c, unsigned char (*f)(unsigned short, struct cpu *, int) )
{
    unsigned short addr;
    addr  = cpu_addr( 0xFF & (c->X + inst->op0), 0, CPU_RD);
    addr |= cpu_addr( 0xFF & (c->X + inst->op0 + 1), 0, CPU_RD) << 8;
    f(addr, c, IS_ADDR);
    c->PC+=2;
}

int inst_izy(struct instruction *inst, struct cpu *c, unsigned char (*f)(unsigned short, struct cpu *, int) )
{
    unsigned short addr, old_addr;
    old_addr  = cpu_addr( 0xFF & inst->op0, 0, CPU_RD);
    old_addr |= cpu_addr( 0xFF & (inst->op0 + 1), 0, CPU_RD) << 8;
    addr = old_addr + c->Y;
    f(addr, c, IS_ADDR);
    c->PC+=2;
    return diff_page(old_addr, addr);
}




void exec_one_inst(struct cpu *c)
{
    
    struct instruction inst;
    unsigned short addr, old_PC;
    unsigned char data;
    char s_data;
    int page_crossed;

    addr = c->PC;
    inst.opcode = cpu_addr(addr, 0, CPU_RD);
    inst.op0 = cpu_addr(addr+1, 0, CPU_RD);
    inst.op1 = cpu_addr(addr+2, 0, CPU_RD);
    inst.op2 = cpu_addr(addr+3, 0, CPU_RD);

#ifdef DEBUG
    printf("CYC:%4lu\taddr:%x, op:%2x, op0:%2x, op1:%2x, op2:%2x\n",c->cycle, addr, inst.opcode, inst.op0, inst.op1, inst.op2);
#endif

    switch(inst.opcode){
    case 0x00:  //BRK 7
        push(c, c->PC & 0xFF);
        push(c, (c->PC >> 8) & 0xFF);
        SET_B(c->P);
        SET_U(c->P);
        push(c, c->P | BITMASK_U | BITMASK_B );
        c->PC = cpu_read16(IRQ_VEC);
        SET_I(c->P);
        c->cycle+=7;
        break;
    case 0x01:  //ORA izx 6
        inst_izx(&inst, c, exec_ORA);
        c->cycle+=6;
        break;
    case 0x03:  //*SLO izx 8
        inst_izx(&inst, c, exec_SLO);
        c->cycle+=8;
        break;
    case 0x04:  //NOP zp 3
        inst_zp(&inst, c, exec_NOP);
        c->cycle+=3;
        break;
    case 0x05:  //ORA zp 3
        inst_zp(&inst, c, exec_ORA);
        c->cycle+=3;
        break;
    case 0x06:  //ASL zp 5
        inst_zp(&inst, c, exec_ASL);
        c->cycle+=5;
        break;
    case 0x07:  //*SLO zp 5
        inst_zp(&inst, c, exec_SLO);
        c->cycle+=5;
        break;
    case 0x08:  //PHP 3
        push(c, c->P | BITMASK_B | BITMASK_U);
        c->PC++;
        c->cycle+=3;
        break;
    case 0x09:  //ORA imm
        inst_imm(&inst, c, exec_ORA);
        c->cycle+=2;
        break;
    case 0x0a:  //ASL 2
        inst_A(&inst, c, exec_ASL);
        c->cycle+=2;
        break;
    case 0x0b:  //ANC imm 2
        break;
    case 0x0c:  //*NOP abs 4
        inst_abs(&inst, c, exec_NOP);
        c->cycle+=4;
        break;
    case 0x0d:  //ORA abs 4
        inst_abs(&inst, c, exec_ORA);
        c->cycle+=4;
        break;
    case 0x0e:  //ASL abs 6
        inst_abs(&inst, c, exec_ASL);
        c->cycle+=6;
        break;
    case 0x0f:  //*SLO abs 6
        inst_abs(&inst, c, exec_SLO);
        c->cycle+=6;
        break;

    case 0x10:  //BPL rel
        c->PC += 2;
        if(!TST_N(c->P)){
            c->cycle+=1;
            old_PC = c->PC;
            c->PC += (signed char)inst.op0;
            if(diff_page(old_PC,c->PC))
                c->cycle+=2;
        }
        c->cycle+=2;
        break;
    case 0x11:  //ORA izy 5*
        page_crossed = inst_izy(&inst, c, exec_ORA);
        if(page_crossed)
            c->cycle++;
        c->cycle+=5;
        break;
    case 0x12:  //KIL
        break;
    case 0x13:  //*SLO izy 8
        inst_izy(&inst, c, exec_SLO);
        c->cycle+=8;
        break;
    case 0x14:  //*NOP zpx 4
        inst_zpx(&inst, c, exec_NOP);
        c->cycle+=4;
        break;
    case 0x15:  //ORA zpx 4
        inst_zpx(&inst, c, exec_ORA);
        c->cycle+=4;
        break;
    case 0x16:  //ASL zpx 6
        inst_zpx(&inst, c, exec_ASL);
        c->cycle+=6;
        break;
    case 0x17:  //*SLO zpx 6
        inst_zpx(&inst, c, exec_SLO);
        c->cycle+=6;
        break;
    case 0x18:  //CLC 2
        CLR_C(c->P);
        c->PC++;
        c->cycle+=2;
        break;
    case 0x19:  //ORA aby 4*
        page_crossed = inst_aby(&inst, c, exec_ORA);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0x1a:  //*NOP 2
        c->PC++;
        c->cycle+=2;
        break;
    case 0x1b:  //SLO aby 7
        inst_aby(&inst, c, exec_SLO);
        c->cycle+=7;
        break;
    case 0x1c:  //*NOP abx 4*
        page_crossed = inst_abx(&inst, c, exec_NOP);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0x1d:  //ORA abx 4*
        page_crossed = inst_abx(&inst, c, exec_ORA);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0x1e:  //ASL abx 7
        inst_abx(&inst, c, exec_ASL);
        c->cycle+=7;
        break;
    case 0x1f:  //*SLO abx 7
        inst_abx(&inst, c, exec_SLO);
        c->cycle+=7;
        break;

    case 0x20:  //JSR abs 6
        c->PC += 3;
        push(c, ( (c->PC-1) >> 8) & 0xFF);
        push(c, (c->PC-1) & 0xFF);
        c->PC = inst.op0 | (inst.op1<<8);
        c->cycle+=6;
        break;
    case 0x21:  //AND izx 6
        inst_izx(&inst, c, exec_AND);
        c->cycle+=6;
        break;
    case 0x22:  //
        break;
    case 0x23:  //*RLA izx 8
        inst_izx(&inst, c, exec_RLA);
        c->cycle+=8;
        break;
    case 0x24:  //BIT zp 3
        inst_zp(&inst, c, exec_BIT);
        c->cycle+=3;
        break;
    case 0x25:  //AND zp 3
        inst_zp(&inst, c, exec_AND);
        c->cycle+=3;
        break;
    case 0x26:  //ROL zp 5
        inst_zp(&inst, c, exec_ROL);
        c->cycle+=5;
        break;
    case 0x27:  //*RLA zp 5
        inst_zp(&inst, c, exec_RLA);
        c->cycle+=5;
        break;
    case 0x28:  //PLP 4
        data = pop(c);
        WRT_B(data, TST_B(c->P));
        WRT_U(data, TST_U(c->P));
        c->P = data;
        c->PC++;
        c->cycle+=4;
        break;
    case 0x29:  //AND imm 2
        inst_imm(&inst, c, exec_AND);
        c->cycle+=2;
        break;
    case 0x2a:  //ROL 2
        inst_A(&inst, c, exec_ROL);
        c->cycle+=2;
        break;
    case 0x2b:  //
        break;
    case 0x2c:  //BIT abs 4
        inst_abs(&inst, c, exec_BIT);
        c->cycle+=4;
        break;
    case 0x2d:  //AND abs 4
        inst_abs(&inst, c, exec_AND);
        c->cycle+=4;
        break;
    case 0x2e:  //ROL abs 6
        inst_abs(&inst, c, exec_ROL);
        c->cycle+=6;
        break;
    case 0x2f:  //*RLA abs 6
        inst_abs(&inst, c, exec_RLA);
        c->cycle+=6;
        break;

    case 0x30:  //BMI rel 2*
        c->PC+=2;
        if(TST_N(c->P)){
            c->cycle++;
            old_PC = c->PC;
            c->PC+=inst.op0;
            if(diff_page(old_PC, c->PC))
                c->cycle+=2;
        }
        c->cycle+=2;
        break;
    case 0x31:  //AND izy 5*
        page_crossed = inst_izy(&inst, c, exec_AND);
        if(page_crossed)
            c->cycle++;
        c->cycle+=5;
        break;
    case 0x32:  //
        break;
    case 0x33:  //*RLA izy 8
        inst_izy(&inst, c, exec_RLA);
        c->cycle+=8;
        break;
    case 0x34:  //*NOP zpx
        inst_zpx(&inst, c, exec_NOP);
        c->cycle+=4;
        break;
    case 0x35:  //AND zpx 4
        inst_zpx(&inst, c, exec_AND);
        c->cycle+=4;
        break;
    case 0x36:  //ROL zpx 6
        inst_zpx(&inst, c, exec_ROL);
        c->cycle+=6;
        break;
    case 0x37:  //*RLA zpx 6
        inst_zpx(&inst, c, exec_RLA);
        c->cycle+=6;
        break;
    case 0x38:  //SEC 2
        SET_C(c->P);
        c->PC++;
        c->cycle+=2;
        break;
    case 0x39:  //AND aby 4*
        page_crossed = inst_aby(&inst, c, exec_AND);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0x3a:  //*NOP 2
        c->PC++;
        c->cycle+=2;
        break;
    case 0x3b:  //*RLA aby 7
        inst_aby(&inst, c, exec_RLA);
        c->cycle+=7;
        break;
    case 0x3c:  //*NOP abx
        page_crossed = inst_abx(&inst, c, exec_NOP);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0x3d:  //AND abx 4
        page_crossed = inst_abx(&inst, c, exec_AND);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0x3e:  //ROL abx 7
        inst_abx(&inst, c, exec_ROL);
        c->cycle+=7;
        break;
    case 0x3f:  //*RLA abx 7
        inst_abx(&inst, c, exec_RLA);
        c->cycle+=7;
        break;

    case 0x40:  //RTI 6
        data = pop(c);
        WRT_B(data, TST_B(c->P));
        WRT_U(data, TST_U(c->P));
        c->P = data;
        c->PC = pop(c) | (pop(c)<<8);
        c->cycle+=6;
        break;
    case 0x41:  //EOR izx 6
        inst_izx(&inst, c, exec_EOR);
        c->cycle+=6;
        break;
    case 0x42:  //
        break;
    case 0x43:  //*SRE izx 8
        inst_izx(&inst, c, exec_SRE);
        c->cycle+=8;
        break;
    case 0x44:  //*NOP zp
        inst_zp(&inst, c, exec_NOP);
        c->cycle+=3;
        break;
    case 0x45:  //EOR zp 3
        inst_zp(&inst, c, exec_EOR);
        c->cycle+=3;
        break;
    case 0x46:  //LSR zp 5
        inst_zp(&inst, c, exec_LSR);
        c->cycle+=5;
        break;
    case 0x47:  //*SRE zp 5
        inst_zp(&inst, c, exec_SRE);
        c->cycle+=5;
        break;
    case 0x48:  //PHA 3
        push(c, c->A);
        c->PC++;
        c->cycle+=3;
        break;
    case 0x49:  //EOR imm 2
        inst_imm(&inst, c, exec_EOR);
        c->cycle+=2;
        break;
    case 0x4a:  //LSR 2
        inst_A(&inst, c, exec_LSR);
        c->cycle+=2;
        break;
    case 0x4b:  //
        break;
    case 0x4c:  //JMP abs 3
        c->PC = (inst.op1<<8) | inst.op0;
        c->cycle += 3;
        break;
    case 0x4d:  //EOR abs 4
        inst_abs(&inst, c, exec_EOR);
        c->cycle+=4;
        break;
    case 0x4e:  //LSR abs 6
        inst_abs(&inst, c, exec_LSR);
        c->cycle+=6;
        break;
    case 0x4f:  //*SRE abs 6
        inst_abs(&inst, c, exec_SRE);
        c->cycle+=6;
        break;


    case 0x50:  //BVC rel
        c->PC+=2;
        if(!(TST_V(c->P))){
            c->cycle++;
            old_PC = c->PC;
            c->PC+=inst.op0;
            if(diff_page(old_PC, c->PC))
                c->cycle+=2;
        }
        c->cycle+=2;
        break;
    case 0x51:  //EOR izy
        page_crossed = inst_izy(&inst, c, exec_EOR);
        if(page_crossed)
            c->cycle++;
        c->cycle+=5;
        break;
    case 0x52:  //
        break;
    case 0x53:  //*SRE izy 8
        inst_izy(&inst, c, exec_SRE);
        c->cycle+=8;
        break;
    case 0x54:  //*NOP zpx
        inst_zpx(&inst, c, exec_NOP);
        c->cycle+=4;
        break;
    case 0x55:  //EOR zpx
        inst_zpx(&inst, c, exec_EOR);
        c->cycle+=4;
        break;
    case 0x56:  //LSR zpx
        inst_zpx(&inst, c, exec_LSR);
        c->cycle+=6;
        break;
    case 0x57:  //*SRE zpx 6
        inst_zpx(&inst, c, exec_SRE);
        c->cycle+=6;
        break;
    case 0x58:  //CLI 2
        CLR_I(c->P);
        c->PC+=1;
        c->cycle+=2;
        break;
    case 0x59:  //EOR aby
        page_crossed = inst_aby(&inst, c, exec_EOR);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0x5a:  //*NOP 2
        c->PC++;
        c->cycle+=2;
        break;
    case 0x5b:  //*SRE aby 7
        inst_aby(&inst, c, exec_SRE);
        c->cycle+=7;
        break;
    case 0x5c:  //*NOP abx
        page_crossed = inst_abx(&inst, c, exec_NOP);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0x5d:  //EOR abx
        page_crossed = inst_abx(&inst, c, exec_EOR);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0x5e:  //LSR abx
        inst_abx(&inst, c, exec_LSR);
        c->cycle+=7;
        break;
    case 0x5f:  //*SRE abx 7
        inst_abx(&inst, c, exec_SRE);
        c->cycle+=7;
        break;

    case 0x60:  //RTS
        c->PC = pop(c)|(pop(c)<<8);
        c->PC++;
        c->cycle+=6;
        break;
    case 0x61:  //ADC izx
        inst_izx(&inst, c, exec_ADC);
        c->cycle+=6;
        break;
    case 0x62:  //
        break;
    case 0x63:  //*RRA izx 8
        inst_izx(&inst, c, exec_RRA);
        c->cycle+=8;
        break;
    case 0x64:  //*NOP zp
        inst_zp(&inst, c, exec_NOP);
        c->cycle+=3;
        break;
    case 0x65:  //ADC zp
        inst_zp(&inst, c, exec_ADC);
        c->cycle+=3;
        break;
    case 0x66:  //ROR zp
        inst_zp(&inst, c, exec_ROR);
        c->cycle+=5;
        break;
    case 0x67:  //*RRA zp 5
        inst_zp(&inst, c, exec_RRA);
        c->cycle+=5;
        break;
    case 0x68:  //PLA
        c->A = pop(c);
        WRT_N(c->P, c->A & 0x80);
        WRT_Z(c->P, c->A==0);
        c->PC++;
        c->cycle+=4;
        break;
    case 0x69:  //ADC imm
        inst_imm(&inst, c, exec_ADC);
        c->cycle+=2;
        break;
    case 0x6a:  //ROR
        inst_A(&inst, c, exec_ROR);
        c->cycle+=2;
        break;
    case 0x6b:  //
        break;
    case 0x6c:  //JMP ind
        addr = inst.op0 | (inst.op1 << 8);
        c->PC = cpu_addr(addr, 0, CPU_RD);
        if((addr & 0xFF)==0xFF){
            addr &= 0xFF00;
        }else{
            addr++;
        }
        c->PC |= cpu_addr( addr, 0, CPU_RD)<<8;
        c->cycle+=5;
        break;
    case 0x6d:  //ADC abs
        inst_abs(&inst, c, exec_ADC);
        c->cycle+=4;
        break;
    case 0x6e:  //ROR abs
        inst_abs(&inst, c, exec_ROR);
        c->cycle+=6;
        break;
    case 0x6f:  //*RRA abs 6
        inst_abs(&inst, c, exec_RRA);
        c->cycle+=6;
        break;


    case 0x70:  //BVS rel
        c->PC+=2;
        if(TST_V(c->P)){
            c->cycle++;
            old_PC = c->PC;
            c->PC+=(char)inst.op0;
            if(diff_page(old_PC, c->PC))
                c->cycle+=2;
        }
        c->cycle+=2;
        break;
    case 0x71:  //ADC izy
        page_crossed = inst_izy(&inst, c, exec_ADC);
        if(page_crossed)
            c->cycle++;
        c->cycle+=5;
        break;
    case 0x72:  //
        break;
    case 0x73:  //*RRA izy 8
        inst_izy(&inst, c, exec_RRA);
        c->cycle+=8;
        break;
    case 0x74:  //*NOP zpx
        inst_zpx(&inst, c, exec_NOP);
        c->cycle+=4;
        break;
    case 0x75:  //ADC zpx
        inst_zpx(&inst, c, exec_ADC);
        c->cycle+=4;
        break;
    case 0x76:  //ROR zpx
        inst_zpx(&inst, c, exec_ROR);
        c->cycle+=6;
        break;
    case 0x77:  //*RRA zpx 6
        inst_zpx(&inst, c, exec_RRA);
        c->cycle+=6;
        break;
    case 0x78:  //SEI
        SET_I(c->P);
        c->PC++;
        c->cycle+=2;
        break;
    case 0x79:  //ADC aby
        page_crossed = inst_aby(&inst, c, exec_ADC);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0x7a:  //*NOP 2
        c->PC++;
        c->cycle+=2;
        break;
    case 0x7b:  //*RRA aby 7
        inst_aby(&inst, c, exec_RRA);
        c->cycle+=7;
        break;
    case 0x7c:  //*NOP abx
        page_crossed = inst_abx(&inst, c, exec_NOP);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0x7d:  //ADC abx
        page_crossed = inst_abx(&inst, c, exec_ADC);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0x7e:  //ROR abx
        inst_abx(&inst, c, exec_ROR);
        c->cycle+=7;
        break;
    case 0x7f:  //RRA abx 7
        inst_abx(&inst, c, exec_RRA);
        c->cycle+=7;
        break;


    case 0x80:  //*NOP imm
        inst_imm(&inst, c, exec_NOP);
        c->cycle+=2;
        break;
    case 0x81:  //STA izx
        inst_izx(&inst, c, exec_STA);
        c->cycle+=6;
        break;
    case 0x82:  //*NOP imm
        inst_imm(&inst, c, exec_NOP);
        c->cycle+=2;
        break;
    case 0x83:  //*SAX izx
        inst_izx(&inst, c, exec_SAX);
        c->cycle+=6;
        break;
    case 0x84:  //STY zp
        inst_zp(&inst, c, exec_STY);
        c->cycle+=3;
        break;
    case 0x85:  //STA zp
        inst_zp(&inst, c, exec_STA);
        c->cycle+=3;
        break;
    case 0x86:  //STX zp
        inst_zp(&inst, c, exec_STX);
        c->cycle+=3;
        break;
    case 0x87:  //*SAX zp
        inst_zp(&inst, c, exec_SAX);
        c->cycle+=3;
        break;
    case 0x88:  //DEY
        c->Y--;
        WRT_N(c->P, c->Y & 0x80);
        WRT_Z(c->P, c->Y==0);
        c->PC+=1;
        c->cycle+=2;
        break;
    case 0x89:  //*NOP imm
        inst_imm(&inst, c, exec_NOP);
        c->cycle+=2;
        break;
    case 0x8a:  //TXA
        c->A = c->X;
        WRT_N(c->P, c->A & 0x80);
        WRT_Z(c->P, c->A==0);
        c->PC++;
        c->cycle+=2;
        break;
    case 0x8b:  //
        break;
    case 0x8c:  //STY abs
        inst_abs(&inst, c, exec_STY);
        c->cycle+=4;
        break;
    case 0x8d:  //STA abs
        inst_abs(&inst, c, exec_STA);
        c->cycle+=4;
        break;
    case 0x8e:  //STX abs
        inst_abs(&inst, c, exec_STX);
        c->cycle+=4;
        break;
    case 0x8f:  //*SAX abs
        inst_abs(&inst, c, exec_SAX);
        c->cycle+=4;
        break;


    case 0x90:  //BCC rel
        c->PC+=2;
        if(!TST_C(c->P)){
            c->cycle++;
            old_PC = c->PC;
            c->PC+=(char)inst.op0;
            if(diff_page(old_PC, c->PC))
                c->cycle+=2;
        }
        c->cycle+=2;
        break;
    case 0x91:  //STA izy
        inst_izy(&inst, c, exec_STA);
        c->cycle+=6;
        break;
    case 0x92:  //
        break;
    case 0x93:  //
        break;
    case 0x94:  //STY zpx
        inst_zpx(&inst, c, exec_STY);
        c->cycle+=4;
        break;
    case 0x95:  //STA zpx
        inst_zpx(&inst, c, exec_STA);
        c->cycle+=4;
        break;
    case 0x96:  //STX zpy
        inst_zpy(&inst, c, exec_STX);
        c->cycle+=4;
        break;
    case 0x97:  //*SAX zpy
        inst_zpy(&inst, c, exec_SAX);
        c->cycle+=4;
        break;
    case 0x98:  //TYA
        c->A = c->Y;
        WRT_N(c->P, c->A & 0x80);
        WRT_Z(c->P, c->A==0);
        c->PC++;
        c->cycle+=2;
        break;
    case 0x99:  //STA aby
        inst_aby(&inst, c, exec_STA);
        c->cycle+=5;
        break;
    case 0x9a:  //TXS
        c->SP = c->X;
        c->PC++;
        c->cycle+=2;
        break;
    case 0x9b:  //
        break;
    case 0x9c:  //
        break;
    case 0x9d:  //STA abx
        inst_abx(&inst, c, exec_STA);
        c->cycle+=5;
        break;
    case 0x9e:  //
        break;
    case 0x9f:  //
        break;


    case 0xa0:  //LDY imm
        inst_imm(&inst, c, exec_LDY);
        c->cycle+=2;
        break;
    case 0xa1:  //LDA izx
        inst_izx(&inst, c, exec_LDA);
        c->cycle+=6;
        break;
    case 0xa2:  //LDX imm
        inst_imm(&inst, c, exec_LDX);
        c->cycle+=2;
        break;
    case 0xa3:  //*LAX izx
        inst_izx(&inst, c, exec_LAX);
        c->cycle+=6;
        break;
    case 0xa4:  //LDY zp
        inst_zp(&inst, c, exec_LDY);
        c->cycle+=3;
        break;
    case 0xa5:  //LDA zp
        inst_zp(&inst, c, exec_LDA);
        c->cycle+=3;
        break;
    case 0xa6:  //LDX zp
        inst_zp(&inst, c, exec_LDX);
        c->cycle+=3;
        break;
    case 0xa7:  //*LAX zp
        inst_zp(&inst, c, exec_LAX);
        c->cycle+=3;
        break;
    case 0xa8:  //TAY
        c->Y = c->A;
        WRT_N(c->P, c->Y & 0x80);
        WRT_Z(c->P, c->Y==0);
        c->PC++;
        c->cycle+=2;
        break;
    case 0xa9:  //LDA imm
        inst_imm(&inst, c, exec_LDA);
        c->cycle+=2;
        break;
    case 0xaa:  //TAX
        c->X = c->A;
        WRT_N(c->P, c->X & 0x80);
        WRT_Z(c->P, c->X==0);
        c->PC++;
        c->cycle+=2;
        break;
    case 0xab:  //*LAX imm
        inst_imm(&inst, c, exec_LAX);
        c->cycle+=2;
        break;
    case 0xac:  //LDY abs
        inst_abs(&inst, c, exec_LDY);
        c->cycle+=4;
        break;
    case 0xad:  //LDA abs
        inst_abs(&inst, c, exec_LDA);
        c->cycle+=4;
        break;
    case 0xae:  //LDX abs
        inst_abs(&inst, c, exec_LDX);
        c->cycle+=4;
        break;
    case 0xaf:  //*LAX abs
        inst_abs(&inst, c, exec_LAX);
        c->cycle+=4;
        break;


    case 0xb0:  //BCS rel
        c->PC+=2;
        if(TST_C(c->P)){
            c->cycle++;
            old_PC = c->PC;
            c->PC+=(char)inst.op0;
            if(diff_page(old_PC, c->PC))
                c->cycle+=2;
        }
        c->cycle+=2;
        break;
    case 0xb1:  //LDA izy
        page_crossed = inst_izy(&inst, c, exec_LDA);
        if(page_crossed)
            c->cycle++;
        c->cycle+=5;
        break;
    case 0xb2:  //
        break;
    case 0xb3:  //*LAX izy
        page_crossed = inst_izy(&inst, c, exec_LAX);
        if(page_crossed)
            c->cycle++;
        c->cycle+=5;
        break;
    case 0xb4:  //LDY zpx
        inst_zpx(&inst, c, exec_LDY);
        c->cycle+=4;
        break;
    case 0xb5:  //LDA zpx
        inst_zpx(&inst, c, exec_LDA);
        c->cycle+=4;
        break;
    case 0xb6:  //LDX zpy
        inst_zpy(&inst, c, exec_LDX);
        c->cycle+=4;
        break;
    case 0xb7:  //*LAX zpy
        inst_zpy(&inst, c, exec_LAX);
        c->cycle+=4;
        break;
    case 0xb8:  //CLV
        CLR_V(c->P);
        c->PC+=1;
        c->cycle+=2;
        break;
    case 0xb9:  //LDA aby
        page_crossed = inst_aby(&inst, c, exec_LDA);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0xba:  //TSX
        c->X = c->SP;
        WRT_N(c->P, c->X & 0x80);
        WRT_Z(c->P, c->X==0);
        c->PC++;
        c->cycle+=2;
        break;
    case 0xbb:  //
        break;
    case 0xbc:  //LDY abx
        page_crossed = inst_abx(&inst, c, exec_LDY);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0xbd:  //LDA abx
        page_crossed = inst_abx(&inst, c, exec_LDA);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0xbe:  //LDX aby
        page_crossed = inst_aby(&inst, c, exec_LDX);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0xbf:  //*LAX aby
        page_crossed = inst_aby(&inst, c, exec_LAX);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;


    case 0xc0:  //CPY imm
        inst_imm(&inst, c, exec_CPY);
        c->cycle+=2;
        break;
    case 0xc1:  //CMP izx
        inst_izx(&inst, c, exec_CMP);
        c->cycle+=6;
        break;
    case 0xc2:  //*NOP imm
        inst_imm(&inst, c, exec_NOP);
        c->cycle+=2;
        break;
    case 0xc3:  //*DCP izx
        inst_izx(&inst, c, exec_DCP);
        c->cycle+=8;
        break;
    case 0xc4:  //CPY zp
        inst_zp(&inst, c, exec_CPY);
        c->cycle+=3;
        break;
    case 0xc5:  //CMP zp
        inst_zp(&inst, c, exec_CMP);
        c->cycle+=3;
        break;
    case 0xc6:  //DEC zp
        inst_zp(&inst, c, exec_DEC);
        c->cycle+=5;
        break;
    case 0xc7:  //*DCP zp
        inst_zp(&inst, c, exec_DCP);
        c->cycle+=5;
        break;
    case 0xc8:  //INY
        c->Y++;
        WRT_N(c->P, c->Y & 0x80);
        WRT_Z(c->P, c->Y==0);
        c->PC++;
        c->cycle+=2;
        break;
    case 0xc9:  //CMP imm
        inst_imm(&inst, c, exec_CMP);
        c->cycle+=2;
        break;
    case 0xca:  //DEX
        c->X--;
        WRT_N(c->P, c->X & 0x80);
        WRT_Z(c->P, c->X==0);
        c->PC++;
        c->cycle+=2;
        break;
    case 0xcb:  //
        break;
    case 0xcc:  //CPY abs
        inst_abs(&inst, c, exec_CPY);
        c->cycle+=4;
        break;
    case 0xcd:  //CMP abs
        inst_abs(&inst, c, exec_CMP);
        c->cycle+=4;
        break;
    case 0xce:  //DEC abs
        inst_abs(&inst, c, exec_DEC);
        c->cycle+=6;
        break;
    case 0xcf:  //*DCP abs
        inst_abs(&inst, c, exec_DCP);
        c->cycle+=6;
        break;

    case 0xd0:  //BNE rel
        c->PC+=2;
        if(!TST_Z(c->P)){
            c->cycle++;
            old_PC = c->PC;
            c->PC+=(char)inst.op0;
            if(diff_page(old_PC,c->PC))
                c->cycle+=2;
        }
        c->cycle+=2;
        break;
    case 0xd1:  //CMP izy
        page_crossed = inst_izy(&inst, c, exec_CMP);
        if(page_crossed)
            c->cycle++;
        c->cycle+=5;
        break;
    case 0xd2:  //
        break;
    case 0xd3:  //*DCP izy
        inst_izy(&inst, c, exec_DCP);
        c->cycle+=8;
        break;
    case 0xd4:  //*NOP zpx
        inst_zpx(&inst, c, exec_NOP);
        c->cycle+=4;
        break;
    case 0xd5:  //CMP zpx
        inst_zpx(&inst, c, exec_CMP);
        c->cycle+=4;
        break;
    case 0xd6:  //DEC zpx
        inst_zpx(&inst, c, exec_DEC);
        c->cycle+=6;
        break;
    case 0xd7:  //*DCP zpx
        inst_zpx(&inst, c, exec_DCP);
        c->cycle+=6;
        break;
    case 0xd8:  //CLD
        CLR_D(c->P);
        c->PC++;
        c->cycle+=2;
        break;
    case 0xd9:  //CMP aby
        page_crossed = inst_aby(&inst, c, exec_CMP);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0xda:  //*NOP 2
        c->PC++;
        c->cycle+=2;
        break;
    case 0xdb:  //*DCP aby
        inst_aby(&inst, c, exec_DCP);
        c->cycle+=7;
        break;
    case 0xdc:  //*NOP abx
        page_crossed = inst_abx(&inst, c, exec_NOP);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0xdd:  //CMP abx
        page_crossed = inst_abx(&inst, c, exec_CMP);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0xde:  //DEC abx
        inst_abx(&inst, c, exec_DEC);
        c->cycle+=7;
        break;
    case 0xdf:  //*DCP abx
        inst_abx(&inst, c, exec_DCP);
        c->cycle+=7;
        break;

    case 0xe0:  //CPX imm 2
        inst_imm(&inst, c, exec_CPX);
        c->cycle+=2;
        break;
    case 0xe1:  //SBC izx
        inst_izx(&inst, c, exec_SBC);
        c->cycle+=6;
        break;
    case 0xe2:  //*NOP imm
        inst_imm(&inst, c, exec_NOP);
        c->cycle+=2;
        break;
    case 0xe3:  //*ISC izx
        inst_izx(&inst, c, exec_ISC);
        c->cycle+=8;
        break;
    case 0xe4:  //CPX zp
        inst_zp(&inst, c, exec_CPX);
        c->cycle+=3;
        break;
    case 0xe5:  //SBC zp
        inst_zp(&inst, c, exec_SBC);
        c->cycle+=3;
        break;
    case 0xe6:  //INC zp
        inst_zp(&inst, c, exec_INC);
        c->cycle+=5;
        break;
    case 0xe7:  //*ISC zp
        inst_zp(&inst, c, exec_ISC);
        c->cycle+=5;
        break;
    case 0xe8:  //INX
        c->X++;
        WRT_N(c->P, c->X & 0x80);
        WRT_Z(c->P, c->X==0);
        c->PC++;
        c->cycle+=2;
        break;
    case 0xe9:  //SBC imm
        inst_imm(&inst, c, exec_SBC);
        c->cycle+=2;
        break;
    case 0xea:  //NOP
        c->PC++;
        c->cycle+=2;
        break;
    case 0xeb:  //*SBC imm
        inst_imm(&inst, c, exec_SBC);
        c->cycle+=2;
        break;
    case 0xec:  //CPX abs
        inst_abs(&inst, c, exec_CPX);
        c->cycle+=4;
        break;
    case 0xed:  //SBC abs
        inst_abs(&inst, c, exec_SBC);
        c->cycle+=4;
        break;
    case 0xee:  //INC abs
        inst_abs(&inst, c, exec_INC);
        c->cycle+=6;
        break;
    case 0xef:  //*ISC abs
        inst_abs(&inst, c, exec_ISC);
        c->cycle+=6;
        break;


    case 0xf0:  //BEQ rel
        c->PC+=2;
        if(TST_Z(c->P)){
            c->cycle++;
            old_PC = c->PC;
            c->PC+=(char)inst.op0;
            if(diff_page(old_PC, c->PC))
                c->cycle+=2;
        }
        c->cycle+=2;
        break;
    case 0xf1:  //SBC izy
        page_crossed = inst_izy(&inst, c, exec_SBC);
        if(page_crossed)
            c->cycle++;
        c->cycle+=5;
        break;
    case 0xf2:  //
        break;
    case 0xf3:  //*ISC izy
        inst_izy(&inst, c, exec_ISC);
        c->cycle+=8;
        break;
    case 0xf4:  //*NOP zpx
        inst_zpx(&inst, c, exec_NOP);
        c->cycle+=4;
        break;
    case 0xf5:  //SBC zpx
        inst_zpx(&inst, c, exec_SBC);
        c->cycle+=4;
        break;
    case 0xf6:  //INC zpx
        inst_zpx(&inst, c, exec_INC);
        c->cycle+=6;
        break;
    case 0xf7:  //*ISC zpx
        inst_zpx(&inst, c, exec_ISC);
        c->cycle+=6;
        break;
    case 0xf8:  //SED
        SET_D(c->P);
        c->PC++;
        c->cycle+=2;
        break;
    case 0xf9:  //SBC aby
        page_crossed = inst_aby(&inst, c, exec_SBC);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0xfa:  //*NOP 2
        c->PC++;
        c->cycle+=2;
        break;
    case 0xfb:  //*ISC aby
        inst_aby(&inst, c, exec_ISC);
        c->cycle+=7;
        break;
    case 0xfc:  //*NOP abx
        page_crossed = inst_abx(&inst, c, exec_NOP);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0xfd:  //SBC abx
        page_crossed = inst_abx(&inst, c, exec_SBC);
        if(page_crossed)
            c->cycle++;
        c->cycle+=4;
        break;
    case 0xfe:  //INC abx
        inst_abx(&inst, c, exec_INC);
        c->cycle+=7;
        break;
    case 0xff:  //*ISC abx
        inst_abx(&inst, c, exec_ISC);
        c->cycle+=7;
        break;

    default:
        printf("undefined opcode: %#x\n", inst.opcode);
        break;
    }



}






