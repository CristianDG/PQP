#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static int32_t registers[16];
static int32_t memory[16];

static FILE *input, *output;

uint8_t get_most_significant(uint8_t val){
  return val >> 4;
}

uint8_t get_least_significant(uint8_t val){
  return val & 0x0F;
}

int16_t to_big_endian(uint8_t byte1, uint8_t byte2){
  return byte2 << 8 | byte1;
}

void interpret(uint8_t *code, uint32_t length) {
  uint32_t index = 0;
  uint8_t reg_id_x, reg_id_y;
  bool eq = false, lt = false, gt = false;

  while (index < length) {
    fprintf(output, "0x%08X->", index);
    switch (code[index]) {
    case 0x00:{
      reg_id_x = get_most_significant(code[index+1]);
      int32_t i16 = to_big_endian(code[index+2], code[index+3]);
      registers[reg_id_x] = i16;
      fprintf(output, "MOV R%d=0x%08X", reg_id_x, i16);
    } break;
    case 0x01:{
      reg_id_x = get_most_significant(code[index+1]);
      reg_id_y = get_least_significant(code[index+1]);
      registers[reg_id_x] = registers[reg_id_y];
      fprintf(output, "MOV R%d=R%d=0x%08X", reg_id_x, reg_id_y, registers[reg_id_x]);
    } break;
    case 0x02:{
      //R8=MEM[0x02,0x03,0x04,0x05]=[0x01,0x00,0x00,0x10]
      fprintf(output, "MOV R%d=MEM[]=[]", reg_id_x);
    } break;
    case 0x03:{
      fprintf(output, "MOV MEM[]=R%d=[]", reg_id_y);
    } break;
    case 0x04:{
      reg_id_x = get_most_significant(code[index+1]);
      reg_id_y = get_least_significant(code[index+1]);
      gt = reg_id_x > reg_id_y;
      lt = reg_id_x < reg_id_y;
      eq = reg_id_x == reg_id_y;
      fprintf(output, "CMP R%d<=>R%d(G=%d,L=%d,E=%d)", reg_id_x, reg_id_y, gt, lt, eq);
    } break;
    case 0x05:{
      int32_t i16 = to_big_endian(code[index+2], code[index+3]);
      index += i16;
      fprintf(output, "JMP 0x%08X", index + 4);
    } break;
    case 0x06:{
    } break;
    case 0x07:{
    } break;
    case 0x08:{
    } break;
    case 0x09:{
    } break;
    case 0x0A:{
    } break;
    case 0x0B:{
    } break;
    case 0x0C:{
    } break;
    case 0x0D:{
    } break;
    case 0x0E:{
    } break;
    case 0x0F:{
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

int main(void) {
  input = stdin;
  output = stdout;

  uint8_t code[128] = {0};
  uint32_t byte = 0, n = 0;
  while (fscanf(input, "%X", &byte) == 1) {
    code[n++] = byte;
  }

  interpret(code, n);

  // printando a memória
  fprintf(output, "[");
  for (int i = 0; i < 16; ++i) {
    if (i != 0){
      fprintf(output, ",");
    }
    fprintf(output, "%02X:%d", i, memory[i]);
  }
  fprintf(output, "]\n");

  // printando os registradores
  fprintf(output, "[");
  for (int i = 0; i < 16; ++i) {
    if (i != 0){
      fprintf(output, "|");
    }
    fprintf(output, "R%d=%08X", i, registers[i]);
  }
  fprintf(output, "]\n");


}
