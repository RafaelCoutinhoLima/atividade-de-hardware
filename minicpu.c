#include <stdio.h>
#include <stdint.h>

uint8_t mem[256] = {0};
uint8_t reg[4]   = {0};
uint8_t pc       = 0;
uint8_t zf       = 0;
uint8_t running  = 1;
int     ciclo    = 0;

void fetch(uint8_t *op, uint8_t *a, uint8_t *b) {
    *op = mem[pc];
    *a  = mem[pc + 1];
    *b  = mem[pc + 2];
    pc += 3;
}

void decode_execute(uint8_t op, uint8_t a, uint8_t b) {
    switch (op) {
        case 0x01: reg[a] = mem[b];                   
        break;
        case 0x02: mem[b] = reg[a];                   
        break;
        case 0x03: reg[a] = (reg[a] + reg[b]) & 0xFF; 
        break;
        case 0x04: reg[a] = (reg[a] - reg[b]) & 0xFF; 
        break;
        case 0x05: reg[a] = b;                        
        break;
        case 0x06: zf = (reg[a] == reg[b]) ? 1 : 0;  
        break;
        case 0x07: pc = a;                            
        break;
        case 0x08: if (zf)  pc = a;                   
        break;
        case 0x09: if (!zf) pc = a;                   
        break;
        case 0x0A: running = 0;                       
        break;
    }
}

void trace(uint8_t op, uint8_t a, uint8_t b) {
    static const char *mnemo[] = {
        "???", "LOAD", "STORE", "ADD", "SUB",
        "MOV",  "CMP",  "JMP",  "JZ", "JNZ", "HALT"
    };
    const char *nome = (op <= 0x0A) ? mnemo[op] : "???";
    printf("Ciclo %3d: %-5s 0x%02X, 0x%02X | R0=%3d R1=%3d R2=%3d R3=%3d | PC=0x%02X ZF=%d\n",
           ciclo, nome, a, b,
           reg[0], reg[1], reg[2], reg[3],
           pc, zf);
}

int main(void) {
    mem[0x10] = 3;
    mem[0x11] = 4;

    mem[0x00] = 0x01; mem[0x01] = 0x02; mem[0x02] = 0x10;
    mem[0x03] = 0x01; mem[0x04] = 0x01; mem[0x05] = 0x11;
    mem[0x06] = 0x05; mem[0x07] = 0x00; mem[0x08] = 0x01;
    mem[0x09] = 0x07; mem[0x0A] = 0x12; mem[0x0B] = 0x00;

    mem[0x12] = 0x05; mem[0x13] = 0x02; mem[0x14] = 0x00;
    mem[0x15] = 0x06; mem[0x16] = 0x01; mem[0x17] = 0x02;
    mem[0x18] = 0x08; mem[0x19] = 0x42; mem[0x1A] = 0x00;
    mem[0x1B] = 0x02; mem[0x1C] = 0x00; mem[0x1D] = 0xF0;
    mem[0x1E] = 0x05; mem[0x1F] = 0x00; mem[0x20] = 0x00;
    mem[0x21] = 0x01; mem[0x22] = 0x03; mem[0x23] = 0x10;

    mem[0x24] = 0x01; mem[0x25] = 0x02; mem[0x26] = 0xF0;
    mem[0x27] = 0x03; mem[0x28] = 0x00; mem[0x29] = 0x02;
    mem[0x2A] = 0x05; mem[0x2B] = 0x02; mem[0x2C] = 0x01;
    mem[0x2D] = 0x04; mem[0x2E] = 0x03; mem[0x2F] = 0x02;
    mem[0x30] = 0x05; mem[0x31] = 0x02; mem[0x32] = 0x00;
    mem[0x33] = 0x06; mem[0x34] = 0x03; mem[0x35] = 0x02;
    mem[0x36] = 0x09; mem[0x37] = 0x24; mem[0x38] = 0x00;

    mem[0x39] = 0x05; mem[0x3A] = 0x02; mem[0x3B] = 0x01;
    mem[0x3C] = 0x04; mem[0x3D] = 0x01; mem[0x3E] = 0x02;
    mem[0x3F] = 0x07; mem[0x40] = 0x12; mem[0x41] = 0x00;

    mem[0x42] = 0x02; mem[0x43] = 0x00; mem[0x44] = 0x20;
    mem[0x45] = 0x0A; mem[0x46] = 0x00; mem[0x47] = 0x00;

    printf("=======================================================\n");
    printf(" MiniCPU - Grupo 8: Potenciacao\n");
    printf(" BASE = %d (mem[0x10])   EXP = %d (mem[0x11])\n", mem[0x10], mem[0x11]);
    printf(" Esperado: 3^4 = 81  ->  resultado em mem[0x20]\n");
    printf("=======================================================\n\n");
    printf("%-11s %-5s %-12s %-4s %-4s %-4s %-4s %-7s %s\n",
           "Ciclo", "Instr", "Ops", "R0", "R1", "R2", "R3", "PC", "ZF");
    printf("-------------------------------------------------------\n");

    while (running && pc <= 253) {
        uint8_t op, a, b;
        ciclo++;
        fetch(&op, &a, &b);
        decode_execute(op, a, b);
        trace(op, a, b);
    }

    printf("\n=======================================================\n");
    printf(" Resultado: mem[0x20] = %d\n", mem[0x20]);
    printf("=======================================================\n");

    return 0;
}
