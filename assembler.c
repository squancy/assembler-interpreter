#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct label {
  char* identifier;
  int address;
} Label;

typedef struct reg {
  char* name;
  int value;
} Register;

/*
  Remove comments and unnecessary whitespace & create a symbol table for labels
  Create an array of commands
*/
int* first_pass(const char* code, char* commands[], Label* symbol_table[]) {
  int* sizes = malloc(sizeof(int) * 2);
  char source_code[strlen(code) + 1];
  strncpy(source_code, code, strlen(code) + 1);
  const char s[2] = "\n";

  int k = 0;
  char* token = strtok(source_code, s);
  while (token != NULL) {
    char cur_command[256];
    int i = 0, j = 0, end_ind = strlen(token) - 1;

    while (token[i] == ' ' || token[i] == '\t') {
      i++;
    } 

    while (token[end_ind] == ' ' || token[end_ind] == '\t') {
      end_ind--;
    }

    while (token[i] != ';' && token[i] != '\0' && i <= end_ind) {
      cur_command[j++] = token[i++]; 
    }

    if (j > 0) {
      cur_command[j] = '\0';
      char* cmd = malloc(sizeof(char) * strlen(cur_command) + 1);
      strncpy(cmd, cur_command, strlen(cur_command) + 1);
      commands[k++] = cmd;
    }

    token = strtok(NULL, s);
  }
  
  // Create symbol table
  int t = 0;
  for (int i = 0; i < k; i++) {
    for (int j = 0; j < strlen(commands[i]); j++) {
      if (commands[i][j] == ':') {
        char* id = malloc(sizeof(char) * strlen(commands[i]));
        strncpy(id, commands[i], strlen(commands[i]) - 1);
        Label* symbol = malloc(sizeof(Label));
        symbol->identifier = id;
        symbol->address = i;
        symbol_table[t++] = symbol;
      }
    }
  }
  
  sizes[0] = k;
  sizes[1] = t;
  return sizes;
}

/*
  Search for a register value in the currently initialized array of regs
*/
int get_reg_val(char* name, Register regs[], int len) {
  for (int i = 0; i < len; i++) {
    if (strcmp(regs[i].name, name) == 0) {
      return regs[i].value;
    }
  } 
  return -1;
}

/*
  Update the value of a register if it's already initialized or declare (and initialize) a new one if not
*/
void update_reg(char* name, int val, Register regs[], int* len) {
  int exists = 0;
  for (int i = 0; i < *len; i++) {
    if (strcmp(regs[i].name, name) == 0) {
      regs[i].value = val; 
      exists = 1;
      break;
    }
  }

  if (!exists) {
    char* nm = malloc(sizeof(char) * strlen(name) + 1);
    strncpy(nm, name, strlen(name) + 1);
    Register reg = {.name = nm, .value = val};
    regs[(*len)++] = reg;
  }
}

/*
  Extract at most 2 operands (x, y) for a single command
*/
char** extract_x_y(char* token, const char delim[2]) {
  char** res = malloc(sizeof(char*) * 2);
  char* x = malloc(sizeof(char) * 256);
  char* y = malloc(sizeof(char) * 256);
  int cnt = 0;
  token = strtok(NULL, delim);
  while (token != NULL) {
    if (cnt == 0) {
      strncpy(x, token, strlen(token) + 1);
    } else {
      strncpy(y, token, strlen(token) + 1);
    }
    cnt++;
    token = strtok(NULL, delim);
  }

  if (cnt == 2) {
    x[strlen(x) - 1] = '\0';
  }

  res[0] = x;
  res[1] = y;
  return res;
}

void free_memory(char* x, char* y, char** xy) {
  free(x);
  free(y);
  free(xy);
}

int calc_val(char* token, int vx, int vy) {
  if (strcmp(token, "add") == 0) {
    return vx + vy;
  } else if (strcmp(token, "sub") == 0) {
    return vx - vy;
  } else if (strcmp(token, "mul") == 0) {
    return vx * vy;
  } else if (strcmp(token, "div") == 0) {
    return vx / vy;
  }

  return -1;
}

// Returns true if command is any kind of jump instruction
int is_jump(char* token) {
  char* cmds[7] = {"jmp", "jne", "je", "jge", "jg", "jle", "jl"};
  for (int i = 0; i < 7; i++) {
    if (!strcmp(token, cmds[i])) {
      return 1; 
    }
  }
  
  return 0;
}

/*
  Look up the symbol table for the address of a label
*/
int symbol_lookup(char* label, Label* symbol_table[], int len) {
  for (int i = 0; i < len; i++) {
    if (!strcmp(label, symbol_table[i]->identifier)) {
      return symbol_table[i]->address;
    }
  }

  return -1;
}

// Add the value of a register to the output message
void add_reg_to_msg(char reg_name[], int r, Register regs[], int reg_len, char* res, int* k) {
  reg_name[r] = '\0';
  int val = get_reg_val(reg_name, regs, reg_len);
  char val_str[100];
  sprintf(val_str, "%d", val);
  strcat(res, val_str);
  *k += strlen(val_str);
}

/*
  Parse message
*/
char* parse_msg(char* msg, Register regs[], int reg_len) {
  char* res = malloc(sizeof(char) * 10000);
  res[0] = '\0';
  int is_in_quotes = 0, end_ind = strlen(msg) - 1, k = 0, i = 0;
  int is_in_reg = 0, r = 0, last_reg = 0;
  char reg_name[256];

  while (msg[i] == ' ' || msg[i] == '\t') {
    i++;
  }

  while (msg[end_ind] == ' ' || msg[end_ind] == '\t') {
    end_ind--;
  }

  int y;
  for (y = 0; i <= end_ind; i++, y++) {
    if (!is_in_quotes && msg[i] == '\'') {
      is_in_quotes = 1;
    } else if (is_in_quotes && msg[i] == '\'') {
      is_in_quotes = 0;
    } else if (is_in_quotes) {
      res[k++] = msg[i];
    } else if (!is_in_quotes && (msg[i] == ' ' || msg[i] == '\t')) {
      continue;
    } else if (!is_in_quotes && !is_in_reg && msg[i] == ',') {
      is_in_reg = 1;
    } else if ((is_in_reg && msg[i] != ',') || (y == 0 && msg[i] != '\'')) {
      reg_name[r++] = msg[i];
      if (i == end_ind) {
        last_reg = 1;
      }
      if (!is_in_reg) is_in_reg = 1;
    } else if (is_in_reg && msg[i] == ',') {
      is_in_reg = 0;
      add_reg_to_msg(reg_name, r, regs, reg_len, res, &k);
      r = 0;
    }
  }

  if (last_reg) {
    add_reg_to_msg(reg_name, r, regs, reg_len, res, &k);
  }

  res[k] = '\0';
  return res;
}

/*
  Execute assembly and produce an output
*/
char* second_pass(char* commands[], int k, Label* symbol_table[], int t) {
  char* output = malloc(sizeof(char) * 10000);
  output[0] = '\0';
  Register regs[1000];
  int i = 0, reg_len = 0;
  const char delim[2] = " ";
  int jne = 0, je = 0, jge = 0, jg = 0, jle = 0, jl = 0;
  int return_addr[10000], stack_ptr = 0;

  while (i < k) {
    char cmd_buffer[1000];
    strncpy(cmd_buffer, commands[i], strlen(commands[i]) + 1);
    char* token = strtok(cmd_buffer, delim);
    char* end;
    if (strcmp(token, "mov") == 0) {
      char** xy = extract_x_y(token, delim); 
      char* x = xy[0];
      char* y = xy[1];
      
      int val = (int) strtol(y, &end, 10);
      if (*end != '\0') {
        // register 
        int v = get_reg_val(y, regs, reg_len);
        update_reg(x, v, regs, &reg_len);
      } else {
        // integer 
        update_reg(x, val, regs, &reg_len);
      }

      free_memory(x, y, xy);
    } else if (!strcmp(token, "inc") || !strcmp(token, "dec")) {
      char** xy = extract_x_y(token, delim); 
      char* x = xy[0];      
      char* y = xy[1];
      int v = get_reg_val(x, regs, reg_len);
      int va = v - 1;
      if (!strcmp(token, "inc")) {
        va = v + 1;
      }
      update_reg(x, va, regs, &reg_len);

      free_memory(x, y, xy);
    } else if (!strcmp(token, "add") || !strcmp(token, "sub") || !strcmp(token, "mul") || !strcmp(token, "div")) {
      char** xy = extract_x_y(token, delim); 
      char* x = xy[0];
      char* y = xy[1];
      int vx = get_reg_val(x, regs, reg_len);

      int val = (int) strtol(y, &end, 10);
      if (*end != '\0') {
        // register 
        int vy = get_reg_val(y, regs, reg_len);
        int final = calc_val(token, vx, vy);
        update_reg(x, final, regs, &reg_len);
      } else {
        // integer 
        int final = calc_val(token, vx, val);
        update_reg(x, final, regs, &reg_len);
      }
    } else if (is_jump(token)) {
      char** xy = extract_x_y(token, delim); 
      char* label = xy[0];
      char* y = xy[1];
      
      if (!strcmp(token, "jmp") || (!strcmp(token, "jne") && jne) || (!strcmp(token, "je") && je) ||
          (!strcmp(token, "jge") && jge) || (!strcmp(token, "jg") && jg) || (!strcmp(token, "jle") && jle) ||
          (!strcmp(token, "jl") && jl)) {
        i = symbol_lookup(label, symbol_table, t) - 1;
      } 

      free_memory(label, y, xy);
    } else if (!strcmp(token, "cmp")) {
      char** xy = extract_x_y(token, delim); 
      char* x = xy[0];
      char* y = xy[1];

      int vx = (int) strtol(x, &end, 10);
      if (*end != '\0') {
        // register 
        vx = get_reg_val(x, regs, reg_len);
      }

      int vy = (int) strtol(y, &end, 10);
      if (*end != '\0') {
        // register 
        vy = get_reg_val(y, regs, reg_len);
      }
      
      jne = 0, je = 0, jge = 0, jg = 0, jle = 0, jl = 0;

      if (vx != vy) {
        jne = 1;
      }

      if (vx == vy) {
        je = 1;
      }

      if (vx >= vy) {
        jge = 1;
      }

      if (vx > vy){
        jg = 1;
      }

      if (vx <= vy) {
        jle = 1;
      }

      if (vx < vy) {
        jl = 1;
      }

      free_memory(x, y, xy);
    } else if (!strcmp(token, "call")) {
      char** xy = extract_x_y(token, delim); 
      char* label = xy[0];
      char* y = xy[1];     

      return_addr[stack_ptr++] = i + 1;
      i = symbol_lookup(label, symbol_table, t) - 1;

      free_memory(label, y, xy);
    } else if (!strcmp(token, "ret")) {
      i = return_addr[--stack_ptr] - 1;
    } else if (!strcmp(token, "msg")) {
      char pmes[10000];
      int pl = 0;
      for (int l = 3; l < strlen(commands[i]); l++) {
        pmes[pl++] = commands[i][l];
      }
      pmes[pl] = '\0';
      char* mes = parse_msg(pmes, regs, reg_len);
      strcat(output, mes);
    } else if (!strcmp(token, "end")) {
      for (int i = 0; i < reg_len; i++) {
        free(regs[i].name);
      }

      return output;
    }
    i++;
  }

  for (int i = 0; i < reg_len; i++) {
    free(regs[i].name);
  }
  
  free(output);

  return (char*)-1;
}

char* assembler_interpreter(const char* program) {
  char* commands[10000];
  Label* symbol_table[1000];
  int* sizes = first_pass(program, commands, symbol_table);
  char* output = second_pass(commands, sizes[0], symbol_table, sizes[1]);

  // Free memory
  for (int i = 0; i < sizes[0]; i++) {
    free(commands[i]);
  }

  for (int i = 0; i < sizes[1]; i++) {
    free(symbol_table[i]->identifier);
    free(symbol_table[i]);
  }

  free(sizes);
  return output;
}

int main(void) {
  char* test_program = "mov a, 8   ; instruction mov a, 8\nmov g, 10   ; instruction mov g, 10\ncall func\nmsg 'Random result: ', o\nend\nfunc:\n\tcmp a, g\n\tje exit\n\tmov o, a\n\tadd o, g\n\tret\n; Do nothing\nexit:\n\tmsg 'Do nothing'";
  char* output = assembler_interpreter(test_program);
  return 0;
}
