/*
 * MiniCPU Simulator - Grupo 8: Potenciacao (BASE^EXP)
 *
 * Desafio: Pre-carregar BASE=3 em 0x10 e EXP=4 em 0x11.
 *          Calcular BASE^EXP por multiplicacoes repetidas (sem instrucao MUL).
 *          Cada multiplicacao usa adicoes repetidas.
 *          Resultado em 0x20. Validar: 3^4 = 81.
 *
 * Algoritmo implementado na MiniCPU:
 *   result = 1
 *   loop EXP vezes:
 *       result = result * BASE  (por adicoes repetidas)
 *   mem[0x20] = result
 *
 * Layout de memoria:
 *   0x00-0x0B  Inicializacao (instrucoes)
 *   0x0C-0x0F  Area vazia (gap para preservar os dados abaixo)
 *   0x10       BASE = 3  (dado de entrada)
 *   0x11       EXP  = 4  (dado de entrada)
 *   0x12-0x47  Programa principal (instrucoes)
 *   0x20       RESULTADO (escrito ao final)
 *   0xF0       Temporario interno (copia do resultado antes de multiplicar)
 */

#include <stdio.h>
#include <stdint.h>

/* ── Recursos da MiniCPU ── */
uint8_t mem[256] = {0};
uint8_t reg[4]   = {0};   /* R0, R1, R2, R3 */
uint8_t pc       = 0;
uint8_t zf       = 0;
uint8_t running  = 1;
int     ciclo    = 0;

/* ── Fetch ── */
void fetch(uint8_t *op, uint8_t *a, uint8_t *b) {
    *op = mem[pc];
    *a  = mem[pc + 1];
    *b  = mem[pc + 2];
    pc += 3;
}

/* ── Decode + Execute ── */
void decode_execute(uint8_t op, uint8_t a, uint8_t b) {
    switch (op) {
        case 0x01: reg[a] = mem[b];                         break; /* LOAD  R, addr   */
        case 0x02: mem[b] = reg[a];                         break; /* STORE R, addr   */
        case 0x03: reg[a] = (reg[a] + reg[b]) & 0xFF;       break; /* ADD   R1, R2    */
        case 0x04: reg[a] = (reg[a] - reg[b]) & 0xFF;       break; /* SUB   R1, R2    */
        case 0x05: reg[a] = b;                              break; /* MOV   R, imm    */
        case 0x06: zf = (reg[a] == reg[b]) ? 1 : 0;        break; /* CMP   R1, R2    */
        case 0x07: pc = a;                                  break; /* JMP   addr      */
        case 0x08: if (zf)  pc = a;                         break; /* JZ    addr      */
        case 0x09: if (!zf) pc = a;                         break; /* JNZ   addr      */
        case 0x0A: running = 0;                             break; /* HALT            */
    }
}

/* ── Trace de execucao ── */
void trace(uint8_t op, uint8_t a, uint8_t b) {
    static const char *mnemo[] = {
        "???", "LOAD", "STORE", "ADD", "SUB",
        "MOV", "CMP",  "JMP",  "JZ",  "JNZ", "HALT"
    };
    const char *nome = (op <= 0x0A) ? mnemo[op] : "???";
    printf("Ciclo %2d: %-5s 0x%02X,0x%02X | "
           "R0=%3d R1=%3d R2=%3d R3=%3d | PC=0x%02X ZF=%d\n",
           ciclo, nome, a, b,
           reg[0], reg[1], reg[2], reg[3],
           pc, zf);
}

int main(void) {
    /* ════════════════════════════════════════════
     * 1. Dados de entrada (pre-carregados)
     * ════════════════════════════════════════════ */
    mem[0x10] = 3;   /* BASE */
    mem[0x11] = 4;   /* EXP  */

    /* ════════════════════════════════════════════
     * 2. Programa em opcodes
     *
     * -- Inicializacao (0x00) --
     * ════════════════════════════════════════════ */

    /* 0x00: LOAD R2, 0x10  -> R2 = BASE = 3 */
    mem[0x00] = 0x01; mem[0x01] = 0x02; mem[0x02] = 0x10;

    /* 0x03: LOAD R1, 0x11  -> R1 = EXP = 4 */
    mem[0x03] = 0x01; mem[0x04] = 0x01; mem[0x05] = 0x11;

    /* 0x06: MOV R0, 1      -> R0 = resultado = 1 */
    mem[0x06] = 0x05; mem[0x07] = 0x00; mem[0x08] = 0x01;

    /* 0x09: JMP 0x12       -> pula a area de dados (0x10-0x11) */
    mem[0x09] = 0x07; mem[0x0A] = 0x12; mem[0x0B] = 0x00;

    /* enderecos 0x0C-0x0F: nao utilizados (gap de seguranca)  */
    /* enderecos 0x10-0x11: dados (BASE e EXP)                 */

    /* ────────────────────────────────────────────
     * LOOP EXTERNO em 0x12
     * Repete EXP vezes; a cada iteracao multiplica
     * resultado atual por BASE.
     * ──────────────────────────────────────────── */

    /* 0x12: MOV R2, 0      -> R2 = 0 (para comparacao com CMP) */
    mem[0x12] = 0x05; mem[0x13] = 0x02; mem[0x14] = 0x00;

    /* 0x15: CMP R1, R2     -> testa R1 (EXP counter) == 0 */
    mem[0x15] = 0x06; mem[0x16] = 0x01; mem[0x17] = 0x02;

    /* 0x18: JZ  0x42       -> se EXP == 0, pula para FIM */
    mem[0x18] = 0x08; mem[0x19] = 0x42; mem[0x1A] = 0x00;

    /* 0x1B: STORE R0, 0xF0 -> salva resultado atual em temporario */
    mem[0x1B] = 0x02; mem[0x1C] = 0x00; mem[0x1D] = 0xF0;

    /* 0x1E: MOV R0, 0      -> R0 = 0 (novo acumulador da multiplicacao) */
    mem[0x1E] = 0x05; mem[0x1F] = 0x00; mem[0x20] = 0x00;

    /* 0x21: LOAD R3, 0x10  -> R3 = BASE (contador do loop interno) */
    mem[0x21] = 0x01; mem[0x22] = 0x03; mem[0x23] = 0x10;

    /* ────────────────────────────────────────────
     * LOOP INTERNO em 0x24
     * Soma mem[0xF0] ao acumulador R0, BASE vezes.
     * Resultado: R0 = (resultado_antigo) * BASE
     * ──────────────────────────────────────────── */

    /* 0x24: LOAD R2, 0xF0  -> R2 = resultado antigo */
    mem[0x24] = 0x01; mem[0x25] = 0x02; mem[0x26] = 0xF0;

    /* 0x27: ADD R0, R2     -> R0 += resultado_antigo */
    mem[0x27] = 0x03; mem[0x28] = 0x00; mem[0x29] = 0x02;

    /* 0x2A: MOV R2, 1      -> R2 = 1 (para decrementar contador) */
    mem[0x2A] = 0x05; mem[0x2B] = 0x02; mem[0x2C] = 0x01;

    /* 0x2D: SUB R3, R2     -> R3-- (decrementa contador interno) */
    mem[0x2D] = 0x04; mem[0x2E] = 0x03; mem[0x2F] = 0x02;

    /* 0x30: MOV R2, 0      -> R2 = 0 (para comparacao) */
    mem[0x30] = 0x05; mem[0x31] = 0x02; mem[0x32] = 0x00;

    /* 0x33: CMP R3, R2     -> testa R3 == 0 */
    mem[0x33] = 0x06; mem[0x34] = 0x03; mem[0x35] = 0x02;

    /* 0x36: JNZ 0x24       -> se R3 != 0, continua loop interno */
    mem[0x36] = 0x09; mem[0x37] = 0x24; mem[0x38] = 0x00;

    /* ────────────────────────────────────────────
     * Pos-multiplicacao: decrementa contador externo
     * ──────────────────────────────────────────── */

    /* 0x39: MOV R2, 1      -> R2 = 1 */
    mem[0x39] = 0x05; mem[0x3A] = 0x02; mem[0x3B] = 0x01;

    /* 0x3C: SUB R1, R2     -> R1-- (EXP counter--) */
    mem[0x3C] = 0x04; mem[0x3D] = 0x01; mem[0x3E] = 0x02;

    /* 0x3F: JMP 0x12       -> volta ao inicio do loop externo */
    mem[0x3F] = 0x07; mem[0x40] = 0x12; mem[0x41] = 0x00;

    /* ────────────────────────────────────────────
     * FIM em 0x42
     * ──────────────────────────────────────────── */

    /* 0x42: STORE R0, 0x20 -> grava resultado final em 0x20 */
    mem[0x42] = 0x02; mem[0x43] = 0x00; mem[0x44] = 0x20;

    /* 0x45: HALT           -> para a execucao */
    mem[0x45] = 0x0A; mem[0x46] = 0x00; mem[0x47] = 0x00;

    /* ════════════════════════════════════════════
     * 3. Execucao
     * ════════════════════════════════════════════ */
    printf("=======================================================\n");
    printf(" MiniCPU - Grupo 8: Potenciacao\n");
    printf(" Calcula BASE^EXP por multiplicacoes repetidas (ADD)\n");
    printf(" BASE = %d (mem[0x10])   EXP = %d (mem[0x11])\n",
           mem[0x10], mem[0x11]);
    printf(" Esperado: 3^4 = 81  -> resultado em mem[0x20]\n");
    printf("=======================================================\n\n");

    printf("%-9s %-5s %-9s  %-4s %-4s %-4s %-4s  %-7s %s\n",
           "Ciclo", "Instr", "Ops",
           "R0", "R1", "R2", "R3", "PC", "ZF");
    printf("-------------------------------------------------------\n");

    while (running && pc <= 253) {   /* 253 = ultimo inicio valido de instrucao de 3 bytes */
        uint8_t op, a, b;
        ciclo++;
        fetch(&op, &a, &b);
        decode_execute(op, a, b);
        trace(op, a, b);
    }

    printf("\n=======================================================\n");
    printf(" Resultado final: mem[0x20] = %d\n", mem[0x20]);
    printf(" Validacao: %s\n",
           mem[0x20] == 81 ? "CORRETO! (esperado 81)" : "INCORRETO");
    printf("=======================================================\n");

    return 0;
}
