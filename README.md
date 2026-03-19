# MiniCPU Simulator - Grupo 8: Potenciacao

**Disciplina:** Infraestrutura de Hardware
**Atividade:** Torneio de Processadores - Simule o Ciclo Fetch-Decode-Execute em C

## Integrantes

- Rafael Coutinho Lima
- Pedro Coutinho da Silva
- Guilherme Tolentino Leitao
- Rodrigo Carneiro Paiva

## Desafio (Grupo 8)

Implementar a operacao de **potenciacao** na MiniCPU:

- Pre-carregar `BASE = 3` no endereco `0x10` e `EXP = 4` no endereco `0x11`
- Calcular `BASE^EXP` usando multiplicacoes repetidas (sem instrucao MUL no ISA)
- Cada multiplicacao e feita por adicoes repetidas (sem instrucao ADD entre registradores e imediatos)
- Armazenar o resultado no endereco `0x20`
- **Validacao esperada:** `3^4 = 81`

## Arquitetura da MiniCPU

### Registradores

| Registrador | Descricao |
|---|---|
| R0, R1, R2, R3 | 4 registradores de uso geral (inteiros 0-255) |
| PC | Program Counter (endereco da proxima instrucao, inicia em 0) |
| ZF | Zero Flag (1 se o ultimo CMP resultou em zero, 0 caso contrario) |

### Memoria

256 posicoes (`0x00` a `0xFF`), cada uma armazenando um valor de 0 a 255. Implementada como um array de 256 inteiros (`uint8_t mem[256]`).

### Formato de Instrucao

Cada instrucao ocupa **3 posicoes consecutivas** na memoria:

```
[opcode]   [operand1]   [operand2]
```

### ISA (Conjunto de Instrucoes)

| Opcode | Mnemônico | Operacao | Descricao |
|---|---|---|---|
| 0x01 | LOAD R, addr | R <- MEM[addr] | Carrega valor da memoria para registrador |
| 0x02 | STORE R, addr | MEM[addr] <- R | Armazena registrador na memoria |
| 0x03 | ADD R1, R2 | R1 <- R1 + R2 | Soma dois registradores |
| 0x04 | SUB R1, R2 | R1 <- R1 - R2 | Subtrai R2 de R1 |
| 0x05 | MOV R, imm | R <- imm | Move valor imediato para registrador |
| 0x06 | CMP R1, R2 | flags <- R1 - R2 | Compara dois registradores (atualiza ZF) |
| 0x07 | JMP addr | PC <- addr | Salto incondicional |
| 0x08 | JZ addr | se ZF: PC <- addr | Salta se zero flag ativo |
| 0x09 | JNZ addr | se !ZF: PC <- addr | Salta se zero flag inativo |
| 0x0A | HALT | fim | Para o processador |

## Ciclo Fetch-Decode-Execute

O simulador executa um loop repetindo 3 passos ate o HALT:

**FETCH:** Le 3 posicoes de memoria a partir do PC e avanca PC em 3.

**DECODE:** Identifica a operacao pelo opcode (switch/case).

**EXECUTE:** Realiza a acao: acessa registradores, modifica memoria, atualiza flags ou modifica o PC (saltos).

**Trace:** A cada ciclo, o simulador imprime qual instrucao foi executada, os valores dos registradores R0-R3, o PC e o ZF.

## Layout de Memoria do Programa

| Endereco | Conteudo |
|---|---|
| 0x00 - 0x0B | Inicializacao: carrega BASE, EXP e resultado inicial; salta sobre a area de dados |
| 0x0C - 0x0F | Nao utilizado (gap de seguranca para preservar dados) |
| **0x10** | **BASE = 3 (dado de entrada)** |
| **0x11** | **EXP = 4 (dado de entrada)** |
| 0x12 - 0x1A | Loop externo: verifica contador de EXP, salta para FIM se zero |
| 0x1B - 0x23 | Prepara multiplicacao: salva resultado atual, zera acumulador |
| 0x24 - 0x38 | Loop interno: soma repetida para realizar a multiplicacao |
| 0x39 - 0x41 | Pos-multiplicacao: decrementa contador externo, volta ao loop |
| **0x42 - 0x47** | **FIM: grava resultado em 0x20 e HALT** |
| **0x20** | **Resultado de saida (escrito ao final)** |
| 0xF0 | Temporario interno (resultado antes de cada multiplicacao) |

## Algoritmo

Como o ISA nao possui instrucao de multiplicacao nem de potenciacao, o calculo e feito em dois niveis de laco:

```
resultado = 1
para i de 1 ate EXP:
    temp = resultado
    resultado = 0
    para j de 1 ate BASE:
        resultado = resultado + temp
mem[0x20] = resultado
```

Isso realiza `resultado = resultado * BASE` por adicoes repetidas, e repete o processo `EXP` vezes.

**Execucao:** `1 -> 3 -> 9 -> 27 -> 81` (4 multiplicacoes por 3)

## Como Compilar e Executar

```bash
gcc -o minicpu minicpu.c
./minicpu
```

## Saida Esperada

```
=======================================================
 MiniCPU - Grupo 8: Potenciacao
 BASE = 3 (mem[0x10])   EXP = 4 (mem[0x11])
 Esperado: 3^4 = 81  ->  resultado em mem[0x20]
=======================================================

Ciclo       Instr Ops          R0   R1   R2   R3   PC      ZF
-------------------------------------------------------
Ciclo   1: LOAD  0x02, 0x10 | R0=  0 R1=  0 R2=  3 R3=  0 | PC=0x03 ZF=0
...
Ciclo 129: HALT  0x00, 0x00 | R0= 81 R1=  0 R2=  0 R3=  0 | PC=0x48 ZF=1

=======================================================
 Resultado: mem[0x20] = 81
 Validacao: CORRETO (esperado 81)
=======================================================
```

O simulador executa **129 ciclos** no total para calcular `3^4 = 81`.
