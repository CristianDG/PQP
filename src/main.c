#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static int32_t registers[16];
static uint8_t instructions[16];

static FILE *input, *output;

uint8_t get_most_significant(uint8_t val) { return val >> 4; }

uint8_t get_least_significant(uint8_t val) { return val & 0x0F; }

int16_t to_big_endian(uint8_t byte1, uint8_t byte2) {
  return byte2 << 8 | byte1;
}

void interpret(uint8_t *code, uint32_t length) {
  uint32_t index = 0;
  uint8_t reg_id_x, reg_id_y;
  bool eq = false, lt = false, gt = false;

  while (index < length) {
    fprintf(output, "0x%08X->", index);
    instructions[code[index]] += 1;
    switch (code[index]) {
    case 0x00: {
      reg_id_x = get_most_significant(code[index + 1]);
      int32_t i16 = to_big_endian(code[index + 2], code[index + 3]);
      registers[reg_id_x] = i16;
      fprintf(output, "MOV R%d=0x%08X", reg_id_x, i16);
    } break;
    case 0x01: {
      reg_id_x = get_most_significant(code[index + 1]);
      reg_id_y = get_least_significant(code[index + 1]);
      registers[reg_id_x] = registers[reg_id_y];
      fprintf(output, "MOV R%d=R%d=0x%08X", reg_id_x, reg_id_y,
              registers[reg_id_x]);
    } break;
    case 0x02: {
      reg_id_x = get_most_significant(code[index + 1]);
      reg_id_y = get_least_significant(code[index + 1]);
      // R8=MEM[0x02,0x03,0x04,0x05]=[0x01,0x00,0x00,0x10]

      int16_t val = registers[reg_id_y];
      uint8_t *mem = &code[val];

      registers[reg_id_x] =
          (mem[0] << 12) | (mem[1] << 8) | (mem[2] << 4) | mem[3];
      fprintf(
          output,
          "MOV "
          "R%d=MEM[0x%02X,0x%02X,0x%02X,0x%02X]=[0x%02X,0x%02X,0x%02X,0x%02X]",
          reg_id_x, val, val + 1, val + 2, val + 3, mem[0], mem[1], mem[2],
          mem[3]);

    } break;
    case 0x03: {
      reg_id_x = get_most_significant(code[index + 1]);
      reg_id_y = get_least_significant(code[index + 1]);

      int32_t mem_idx = registers[reg_id_x];
      uint8_t *mem = &code[mem_idx];
      uint8_t *val = (uint8_t *)(&registers[reg_id_y]);
      mem[0] = val[3];
      mem[1] = val[2];
      mem[2] = val[1];
      mem[3] = val[0];

      fprintf(
          output,
          "MOV "
          "MEM[0x%02X,0x%02X,0x%02X,0x%02X]=R%d=[0x%02X,0x%02X,0x%02X,0x%02X]",
          mem_idx, mem_idx + 1, mem_idx + 2, mem_idx + 3, reg_id_y, mem[0],
          mem[1], mem[2], mem[3]);

    } break;
    case 0x04: {
      reg_id_x = get_most_significant(code[index + 1]);
      reg_id_y = get_least_significant(code[index + 1]);
      gt = reg_id_x > reg_id_y;
      lt = reg_id_x < reg_id_y;
      eq = reg_id_x == reg_id_y;
      fprintf(output, "CMP R%d<=>R%d(G=%d,L=%d,E=%d)", reg_id_x, reg_id_y, gt,
              lt, eq);
    } break;
    case 0x05: {
      int32_t i16 = to_big_endian(code[index + 2], code[index + 3]);
      index += i16;
      fprintf(output, "JMP 0x%08X", index + 4);
    } break;
    case 0x06: {
      int32_t i16 = to_big_endian(code[index + 2], code[index + 3]);
      fprintf(output, "JG 0x%08X", i16 + index + 4);
      if (gt) {
        index += i16;
      }
    } break;
    case 0x07: {
      int32_t i16 = to_big_endian(code[index + 2], code[index + 3]);
      fprintf(output, "JL 0x%08X", i16 + index + 4);
      if (lt) {
        index += i16;
      }
    } break;
    case 0x08: {
      int32_t i16 = to_big_endian(code[index + 2], code[index + 3]);
      fprintf(output, "JE 0x%08X", i16 + index + 4);
      if (eq) {
        index += i16;
      }
    } break;
    case 0x09: {
      reg_id_x = get_most_significant(code[index + 1]);
      reg_id_y = get_least_significant(code[index + 1]);
      fprintf(output, "ADD R%d+=R%d=0x%08X+0x%08X=0x%08X", reg_id_x, reg_id_y,
              registers[reg_id_x], registers[reg_id_y],
              registers[reg_id_x] + registers[reg_id_y]);
      registers[reg_id_x] += registers[reg_id_y];
    } break;
    case 0x0A: {
      reg_id_x = get_most_significant(code[index + 1]);
      reg_id_y = get_least_significant(code[index + 1]);
      fprintf(output, "SUB R%d-=R%d=0x%08X-0x%08X=0x%08X", reg_id_x, reg_id_y,
              registers[reg_id_x], registers[reg_id_y],
              registers[reg_id_x] - registers[reg_id_y]);
      registers[reg_id_x] -= registers[reg_id_y];
    } break;
    case 0x0B: {
      reg_id_x = get_most_significant(code[index + 1]);
      reg_id_y = get_least_significant(code[index + 1]);
      fprintf(output, "AND R%d&=R%d=0x%08X&0x%08X=0x%08X", reg_id_x, reg_id_y,
              registers[reg_id_x], registers[reg_id_y],
              registers[reg_id_x] & registers[reg_id_y]);
      registers[reg_id_x] &= registers[reg_id_y];
    } break;
    case 0x0C: {
      reg_id_x = get_most_significant(code[index + 1]);
      reg_id_y = get_least_significant(code[index + 1]);
      fprintf(output, "OR R%d|=R%d=0x%08X|0x%08X=0x%08X", reg_id_x, reg_id_y,
              registers[reg_id_x], registers[reg_id_y],
              registers[reg_id_x] | registers[reg_id_y]);
      registers[reg_id_x] |= registers[reg_id_y];
    } break;
    case 0x0D: {
      reg_id_x = get_most_significant(code[index + 1]);
      reg_id_y = get_least_significant(code[index + 1]);
      fprintf(output, "XOR R%d^=R%d=0x%08X^0x%08X=0x%08X", reg_id_x, reg_id_y,
              registers[reg_id_x], registers[reg_id_y],
              registers[reg_id_x] ^ registers[reg_id_y]);
      registers[reg_id_x] ^= registers[reg_id_y];
    } break;
    case 0x0E: {
      reg_id_x = get_most_significant(code[index + 1]);
      uint8_t qty = code[index + 3];
      fprintf(output, "SAL R%d<<=%d=0x%08X<<%d=0x%08X", reg_id_x, qty,
              registers[reg_id_x], qty, registers[reg_id_x] << qty);
      registers[reg_id_x] = registers[reg_id_x] << qty;
    } break;
    case 0x0F: {
      reg_id_x = get_most_significant(code[index + 1]);
      uint8_t qty = code[index + 3];
      fprintf(output, "SAR R%d>>=%d=0x%08X>>%d=0x%08X", reg_id_x, qty,
              registers[reg_id_x], qty, registers[reg_id_x] >> qty);
      registers[reg_id_x] = registers[reg_id_x] >> qty;
    } break;
    default: {
      fprintf(output, "Unknown code: 0x%02X on line, ignoring...", code[index]);
    }
    }

    fprintf(output, "\n");
    index += 4;
  }
  fprintf(output, "0x%08X->EXIT\n", index);
}

int main(int argc, char *argv[]) {
  input = stdin;
  output = stdout;
  if(argc == 3){
    input = fopen(argv[1], "r");
    output = fopen(argv[2], "w");
  }

  uint8_t code[128] = {0};
  uint32_t byte = 0, n = 0;
  while (fscanf(input, "%X", &byte) == 1) {
    code[n++] = byte;
  }

  interpret(code, n);

  // printando a memória
  fprintf(output, "[");
  for (int i = 0; i < 16; ++i) {
    if (i != 0) {
      fprintf(output, ",");
    }
    fprintf(output, "%02X:%d", i, instructions[i]);
  }
  fprintf(output, "]\n");

  // printando os registradores
  fprintf(output, "[");
  for (int i = 0; i < 16; ++i) {
    if (i != 0) {
      fprintf(output, "|");
    }
    fprintf(output, "R%d=%08X", i, registers[i]);
  }
  fprintf(output, "]\n");
}
