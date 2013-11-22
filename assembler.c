#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

//Author: Martin Borstad

#define N_INFO 32
#define N_OpCode 16
#define N_Func 8
struct _Data {
  char** Name;
  int*   position;
  int    size;
  int    numOfData;
};
typedef struct _Data Data;
struct _Info {
  char* Name;
  int Value;
};
typedef struct _Info Info;
//My opcodes
static Info OpCode[N_OpCode] = {
  {"r", 0},
  {"lw", 0b100011},
  {"sw", 0b101011},
  {"lui", 0b001111},
  {"beq", 0b100},
  {"bne", 0b101},
  {"bltz", 0b1},
  {"blez", 0b110},
  {"j", 0b10},
  {"jr", 0b0},//THIS IS SPECIAL!!!!
  {"jal", 0b11},
  {"ori", 0b001101},
  {"andi", 0b001100},
  {"addi", 0b001000},
  {"addiu", 0b001001},
  {"slti", 0b001010}
};
//My function codes
static Info Func[N_Func] = {
  {"syscall", 0b1100},
  {"add", 0b100000},
  {"sub", 0b100010},
  {"or", 0b100101},
  {"and", 0b100100},
  {"slt", 0b101010},
  {"sll", 0b0},
  {"srl", 0b000010}
};
//My register values (where is that register at?)
static Info Table[N_INFO] = {
  {"$zero", 0},
  {"$at", 1},
  {"$v0", 2},
  {"$v1", 3},
  {"$a0", 4},
  {"$a1", 5},
  {"$a2", 6},
  {"$a3", 7},
  {"$t0", 8},
  {"$t1", 9},
  {"$t2", 10},
  {"$t3", 11},
  {"$t4", 12},
  {"$t5", 13},
  {"$t6", 14},
  {"$t7", 15},
  {"$s0", 16},
  {"$s1", 17},
  {"$s2", 18},
  {"$s3", 19},
  {"$s4", 20},
  {"$s5", 21},
  {"$s6", 22},
  {"$s7", 23},
  {"$t8", 24},
  {"$t9", 25},
  {"$k0", 26},
  {"$k1", 27},
  {"$gp", 28},
  {"$sp", 29},
  {"$fp", 30},
  {"$ra", 31}
}; 

void printData(FILE *stream, Data *dat);
void print_R(FILE *stream, const int rs, const int rt, const int rd, const int shamt, const int fun);
void print_I(FILE *stream, const int op, const int rs, const int rt, const int imm);
void print_J(FILE *stream, const int op, const int imm);
char* convertB(const int i, const int size);
void simplify(FILE *stream1, FILE *stream2);
void undoGet(FILE *stream, char *str);
Data* step2(FILE *stream1, FILE *stream2);
void replace(FILE *stream1, FILE *stream2, Data *dat);
int  power10(int i);
int  strEquals(char* str, char* str2, int length);
int  find(Data *dat, char *c, int size);
void toBinary(FILE *stream1, FILE *stream2);
int  getOp(char *c);
int  getFunc(char *c);
int  getReg(char *c);
int  toInt(char *c);

int main(int argc, char *argv[])
{
  if(argc < 2)
  {
    printf("Not enough inputs!\n");
    return 1;
  }
  else if (strEquals(argv[1], "-symbols", 9))
  {
    FILE *fp = fopen(argv[2],"r");
    FILE *fwrite = fopen("output1.txt","w");
    FILE *s2     = fopen("output2.txt","w");
    simplify(fp, fwrite);
    fclose(fwrite);
    fwrite = fopen("output1.txt", "r");
    Data* dat = step2(fwrite, s2);
    fclose(s2);
    fclose(fp);
    fclose(fwrite);
    printData(stdout, dat);
    free(dat->position);
    for(int i = 0; i < dat->size; i++)
    {
      free(dat->Name[i]);
    }
    free(dat->Name);
    free(dat);
    return 0;
  }
  FILE *fp = fopen(argv[1],"r");
  FILE *fwrite = fopen("output1.txt","w");
  FILE *s2     = fopen("output2.txt","w");
  simplify(fp, fwrite);
  fclose(fwrite);
  fwrite = fopen("output1.txt", "r");
  Data* dat = step2(fwrite, s2);
  fclose(s2);
  fclose(fp);
  fclose(fwrite);
  s2 = fopen("output2.txt", "r");
  FILE *s3 = fopen("output3.txt", "w");
  replace(s2, s3, dat);
  fclose(s2);
  fclose(s3);
  s3 = fopen("output3.txt", "r");
  FILE *out = fopen(argv[2], "w");
  toBinary(s3, out);
  free(dat->position);
  for(int i = 0; i < dat->size; i++)
  {
    free(dat->Name[i]);
  }
  free(dat->Name);
  free(dat);
  fclose(s3);
  fclose(out);
  return 0;
}

int  toInt(char *c)
{
  int k = 0;
  int value = 0;
  int z = 0;
  while (1)
  {
    if(c[k] =='\0')
    {
      break;
    }
    k++;
  }
  while(1)
  {
    if(k == -1)
    {
      break;
    }
    if (c[k] - 48 < 0)
    {
      k--;
      continue;
    }
    value += (c[k] - 48) * power10(z);
    z++;
    k--;
  }
  return value;
}

int  getReg(char *c)
{
  for(int i = 0; i < N_INFO; i++)
  {
    for(int j = 0; j < 6; j++)
    {
      if(Table[i].Name[j] == '\0' || c[j] == ',')
      {
        return Table[i].Value;
      }
      else if(Table[i].Name[j] != c[j])
      {
        break;
      }
    }
  }
  return 0;
}

int  getFunc(char *c)
{
  for(int i = 0; i < N_Func; i++)
  {
    for(int j = 0; j < 10; j++)
    {
      if(Func[i].Name[j] == '\0' || c[j] == ',')
      {
        return Func[i].Value;
      }
      else if(Func[i].Name[j] != c[j])
      {
        break;
      }
    }
  }
  return 0;
}

int  getOp(char *c)
{
  for(int i = 0; i < N_OpCode; i++)
  {
    for(int j = 0; j < 9; j++)
    {
      
      if(OpCode[i].Name[j] != c[j])
      {
        break;
      }
      else if(OpCode[i].Name[j] == '\0')
      {
        return OpCode[i].Value;
      }
    }
  }
  return 0;
}

/**
* Time to make the binary file!
*/
void toBinary(FILE *stream1, FILE *stream2)
{
  char *line = malloc(256);
  int d = 0;
  fscanf(stream1, "%[^\n]", line);
  char c = fgetc(stream1);
  while(isspace(c))
  {
    c = fgetc(stream1);
  }
  ungetc(c, stream1);
  free(line);
  line = malloc(256);
  for(int i = 0; i < 256; i++)
  {
    line[i] = 0;
  }
  fscanf(stream1, "%[^\n]", line);
  //Clear out the .text thing
  while(!feof(stream1))
  {
    if(line[0] == '.')
    {
      d = 1;
      fprintf(stream2, "\n");
    }
    else if(!d && line[0] == 'a')
    {
      //addi addiu
      if(line[3] == 'i' && line[1] == 'd')
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char value[16];
        fscanf(stream1, "%s%s%s%s", ins, reg, reg2, value);
        print_I(stream2, getOp(ins), getReg(reg2), getReg(reg), toInt(value));
        fscanf(stream1, "%[^\n]", line);
      }
      //add
      else if (line[1] == 'd')
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char rd[6];
        fscanf(stream1, "%s%s%s%s", ins, rd, reg, reg2);
        print_R(stream2, getReg(reg), getReg(reg2), getReg(rd), 0, getFunc("add"));
        fscanf(stream1, "%[^\n]", line);
      }
      //andi
      else if(line[3] == 'i')
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char value[16];
        fscanf(stream1, "%s%s%s%s", ins, reg, reg2, value);
        print_I(stream2, getOp(ins), getReg(reg), getReg(reg2), toInt(value));
        fscanf(stream1, "%[^\n]", line);
      }
      //and
      else
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char rd[6];
        fscanf(stream1, "%s%s%s%s", ins, rd, reg, reg2);
        print_R(stream2, getReg(reg), getReg(reg2), getReg(rd), 0, getFunc("and"));
        fscanf(stream1, "%[^\n]", line);
      }
    }
    else if(!d && line[0] == 'b')
    {
      //beq
      if(line[1] == 'e')
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char value[16];
        fscanf(stream1, "%s%s%s%s", ins, reg, reg2, value);
        print_I(stream2, getOp(ins), getReg(reg), getReg(reg2), toInt(value));
        fscanf(stream1, "%[^\n]", line);
      }
      //bne
      else if(line[1] == 'n')
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char value[16];
        fscanf(stream1, "%s%s%s%s", ins, reg, reg2, value);
        print_I(stream2, getOp(ins), getReg(reg), getReg(reg2), toInt(value));
        fscanf(stream1, "%[^\n]", line);
      }
      //bltz
      else if(line[2] == 't')
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char value[16];
        fscanf(stream1, "%s%s%s", ins, reg, value);
        print_I(stream2, getOp(ins), getReg(reg), 0, toInt(value));
        fscanf(stream1, "%[^\n]", line);
      }
      //blez
      else
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char value[16];
        fscanf(stream1, "%s%s%s", ins, reg, value);
        print_I(stream2, getOp(ins), getReg(reg), 0, toInt(value));
        fscanf(stream1, "%[^\n]", line);
      }
    }
    else if (!d && line[0] == 'j')
    {
      //j jal
      if(line[1] != 'r')
      {
        undoGet(stream1, line);
        char ins[6];
        char value[26];
        fscanf(stream1, "%s%s", ins, value);
        print_J(stream2, getOp(ins), toInt(value));
        fscanf(stream1, "%[^\n]", line);
      }
      //jr
      else
      {
        undoGet(stream1, line);
        char ins[6];
        char value[26];
        fscanf(stream1, "%s%s", ins, value);
        print_R(stream2, getReg(value), 0, 0, 0, getFunc(ins));
        fscanf(stream1, "%[^\n]", line);
      }
    }
    else if (!d && line[0] == 's')
    {
      //syscall
      if(line[1] == 'y')
      {
        print_R(stream2, 0, 0, 0, 0, getFunc("syscall"));
      }
      //sub
      else if (line[1] == 'u' && line[2] == 'b')
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char rd[6];
        fscanf(stream1, "%s%s%s%s", ins, rd, reg, reg2);
        print_R(stream2, getReg(reg), getReg(reg2), getReg(rd), 0, getFunc("sub"));
        fscanf(stream1, "%[^\n]", line);
      }
      //slti
      else if(line[3] == 'i')
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char value[16];
        fscanf(stream1, "%s%s%s%s", ins, reg, reg2, value);
        print_I(stream2, getOp(ins), getReg(reg), getReg(reg2), toInt(value));
        fscanf(stream1, "%[^\n]", line);
      }
      //srl
      else if(line[1] == 'r')
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char rd[6];
        fscanf(stream1, "%s%s%s%s", ins, rd, reg, reg2);
        print_R(stream2, 0, getReg(reg), getReg(rd), toInt(reg2), getFunc("srl"));
        fscanf(stream1, "%[^\n]", line);
      }
      //slt
      else if(line[2] == 't')
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char rd[6];
        fscanf(stream1, "%s%s%s%s", ins, rd, reg, reg2);
        print_R(stream2, getReg(reg), getReg(reg2), getReg(rd), 0, getFunc("slt"));
        fscanf(stream1, "%[^\n]", line);
      }
      //sw
      else if(line[1] == 'w')
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[16];
        char rest[5];
        fscanf(stream1, "%s%s%s", ins, reg, reg2);
        char off[13];
        int pos = 0;
        int set = 0;
        for(int z = 0; z < 16; z++)
        {
          if(!set && reg2[z] == '(')
          {
            off[pos] = '\0';
            pos = 0;
            set = 1;
            continue;
          }
          if(reg2[z] == ')')
          {
            rest[pos] = '\0';
            break;
          }
          if(!set && reg2[z] == 'x')
          {
            pos = 0;
            z++;
          }
          if(!set)
          {
            off[pos] = reg2[z];
          }
          else
          {
            rest[pos] = reg2[z];
          }
          pos++;
        }
        print_I(stream2, getOp(ins), getReg(rest), getReg(reg), toInt(off));
        fscanf(stream1, "%[^\n]", line);
      }
      //sll
      else
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char rd[6];
        fscanf(stream1, "%s%s%s%s", ins, rd, reg, reg2);
        print_R(stream2, 0, getReg(reg), getReg(rd), toInt(reg2), getFunc("sll"));
        fscanf(stream1, "%[^\n]", line);
      }
    }
    else if (!d && line[0] == 'l')
    {
      //lui
      if(line[1] == 'u' && line[2] == 'i')
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char value[16];
        fscanf(stream1, "%s%s%s", ins, reg, value);
        print_I(stream2, getOp(ins), 0, getReg(reg), toInt(value));
        fscanf(stream1, "%[^\n]", line);
      }
      //lw
      else
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[16];
        char rest[5];
        fscanf(stream1, "%s%s%s", ins, reg, reg2);
        char off[13];
        int pos = 0;
        int set = 0;
        for(int z = 0; z < 16; z++)
        {
          if(!set && reg2[z] == '(')
          {
            off[pos] = '\0';
            pos = 0;
            set = 1;
            pos--;
          }
          if(reg2[z] == ')')
          {
            rest[pos] = '\0';
            break;
          }
          if(!set && reg2[z] == 'x')
          {
            pos = 0;
            z++;
          }
          if(!set)
          {
            off[pos] = reg2[z];
          }
          else
          {
            rest[pos] = reg2[z];
          }
          pos++;
        }
        print_I(stream2, getOp(ins), getReg(rest), getReg(reg), toInt(off));
        fscanf(stream1, "%[^\n]", line);
      }
    }
    else if (!d && line[0] == 'o')
    {
      //ori
      if(line[2] == 'i')
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char value[16];
        fscanf(stream1, "%s%s%s%s", ins, reg, reg2, value);
        print_I(stream2, getOp(ins), getReg(reg2), getReg(reg), toInt(value));
        fscanf(stream1, "%[^\n]", line);
      }
      //or
      else
      {
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char rd[6];
        fscanf(stream1, "%s%s%s%s", ins, rd, reg, reg2);
        print_R(stream2, getReg(reg), getReg(reg2), getReg(rd), 0, getFunc("or"));
        fscanf(stream1, "%[^\n]", line);
      }
    }
    else if(d)
    {
      undoGet(stream1, line);
      char name[50];
      char type[8];
      fscanf(stream1, "%s%s", name, type);
      if(strEquals(type, ".word", 8))
      {
        char length[14];
        fscanf(stream1, "%s", length);
        fscanf(stream1, "%[^\n]", line);
        int k = 0;
        int j = 0;
        int in = 0;
        int size = 1;
        while(1)
        {
          if(length[k] == ':')
          {
            j = k;
            size = 0;
            int neg = 1;
            while(1)
            {
              if(j <= 0)
              {
                break;
              }
              if(length[k-j] == '-')
              {
                neg = -1;
              }
              else
              {
                in += (length[j - k] - 48) * power10(k - 1);
              }
              j--;
            }
            in *= neg;
            j = k + 1;
            k += 2;
            while(1)
            {
              if(length[k] == '\0')
              {
                break;
              }
              k++;
            }
            k--;
            for(int q = k;q >= j; q--)
            {
              size += (length[q] - 48) * power10(k - q);
            }
            break;
          }
          else if(length[k] == '\0')
          {
            j = k;
            int neg = 1;
            while(1)
            {
              if(k <= 0)
              {
                break;
              }
              if(length[j - k] == '-')
              {
                neg = -1;
              }
              else
              {
                in += (length[j - k] - 48) * power10(k - 1);
              }
              k--;
            }
            in *= neg;
            break;
          }
          k++;
        }
        for(int z = 0; z < size; z++)
        {
          char *f = convertB(in, 32);
          fprintf(stream2, "%s\n", f);
          free(f);
        }
      }
      else
      {
        char tempLine[256];
        fscanf(stream1, "%[^\n]", tempLine );
        int j = 0;
        while(tempLine[j] != '"')
        {
          j++;
        }
        j++;
        int z = 0;
        char* buff[4];
        while(tempLine[j] != '"')
        {
          buff[z] = convertB(tempLine[j], 8);
          z++;
          if(z == 4)
          {
            fprintf(stream2, "%s%s%s%s\n", buff[3], buff[2], buff[1], buff[0]);
            free(buff[3]);
            free(buff[2]);
            free(buff[1]);
            free(buff[0]);
            z = 0;
          }
          j++;
        }
        char *p = convertB(0, 8);
        int g = z;
        for (; z < 4; z++)
        {
          buff[z] = p;
        }
        fprintf(stream2, "%s%s%s%s\n", buff[3], buff[2], buff[1], buff[0]);
        for(int q = 0; q < g; q++)
        {
          free(buff[q]);
        }
        free(p);
        fscanf(stream1, "%[^\n]", tempLine );
      }
    }
    char c = fgetc(stream1);
    while(isspace(c))
    {
      c = fgetc(stream1);
    }
    ungetc(c, stream1);
    free(line);
    line = malloc(256);
    for(int i = 0; i < 256; i++)
    {
      line[i] = 0;
    }
    fscanf(stream1, "%[^\n]", line);
  }
  free(line);
}

int  find(Data *dat, char *c, int size)
{
  for(int i = dat->size - dat->numOfData; i < dat->size; i++)
  {
    for(int j = 0; j < size; j++)
    {
      if(dat->Name[i][j] != c[j] && (dat->Name[i][j] != '\0' || c[j] == '_'))
      {
        break;
      }
      else if (dat->Name[i][j] == '\0')
      {
        return i;
      }
    }
  }
  return -1;
}

/**
*  Time to replace the labels!
*/
void replace(FILE *stream1, FILE *stream2, Data *dat)
{
  char *line = malloc(256);
  for(int i = 0; i < 256; i++)
  {
    line[i] = 0;
  }
  int c = 0;
  fscanf(stream1, "%[^\n]", line );
  int l = 0;
  while(!feof(stream1))
  {
    if(line[0] == '.' && line[1] == 'd' && line[2] == 'a' && line[3] == 't' && line[4] == 'a')
    {
      c = 1;
    }
    int print = 1;
    for(int i = 0; i < 256; i++)
    {
      
      if(line[i] == 'a' && line [i + 1] == 'd' && line[i+2] == 'd' && line[i+3] == 'i' && !c)
      {
        print = 0;
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char value[16];
        fscanf(stream1, "%s%s%s%s", ins, reg, reg2, value);
        int z = find(dat, value, 16);
        if(z == -1)
        {
          fprintf(stream2, "%s   %s %s %s\n", ins, reg, reg2, value);
        }
        else
        {
          fprintf(stream2, "%s   %s %s %d\n", ins, reg, reg2, dat->position[z]);
        }
        fscanf(stream1, "%[^\n]", line);
        break;
      }
      else if (line[i] == 'j' && line[i+1] != 'r' && !c)
      {
        print = 0;
        undoGet(stream1, line);
        char ins[5];
        char value[26];
        fscanf(stream1, "%s%s", ins, value);
        int v = 0;
        for(int j = 0; j < dat->size - dat->numOfData; j++)
        {
          for(int k = 0; k < 26; k++)
          {
            if(dat->Name[j][k] != value[k] && dat->Name[j][k] != '\0')
            {
              break;
            }
            else if (dat->Name[j][k] == '\0')
            {
              v = (int)(((uint32_t)dat->position[j]) >> 2);
              break;
            }
          }
        }
        fprintf(stream2, "%s  %d\n", ins, v);
        fscanf(stream1, "%[^\n]", line);
        break;
      }
      else if (line[i] == 'b' && ((line [i + 1] == 'e' && line[i+2] == 'q') || (line [i + 1] == 'n' && line[i+2] == 'e')) && !c)
      {
        print = 0;
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char reg2[6];
        char value[16];
        fscanf(stream1, "%s%s%s%s", ins, reg, reg2, value);
        int z = -1;
        for(int j = 0; j < dat->size - dat->numOfData + 1; j++)
        {
          for(int k = 0; k < 26; k++)
          {
            if(dat->Name[j][k] != value[k] && dat->Name[j][k] != '\0')
            {
              break;
            }
            else if (dat->Name[j][k] == '\0')
            {
              z = (int)(((uint32_t)dat->position[j] - l) >> 2);
              break;
            }
          }
        }
        if(z == -1)
        {
          fprintf(stream2, "%s   %s %s %s\n", ins, reg, reg2, value);
        }
        else
        {
          fprintf(stream2, "%s   %s %s %d\n", ins, reg, reg2, z);
        }
        fscanf(stream1, "%[^\n]", line);
        break;
      }
      else if (line[i] == 'b' && ((line [i + 1] == 'l' && line[i+2] == 't' && line[i+3] == 'z') || (line [i + 1] == 'l' && line[i+2] == 'e' && line[i+3] == 'z')) && !c)
      {
        print = 0;
        undoGet(stream1, line);
        char ins[5];
        char reg[6];
        char value[16];
        fscanf(stream1, "%s%s%s", ins, reg, value);
        int z = -1;
        for(int j = 0; j < dat->size - dat->numOfData; j++)
        {
          for(int k = 0; k < 26; k++)
          {
            if(dat->Name[j][k] != value[k] && dat->Name[j][k] != '\0')
            {
              break;
            }
            else if (dat->Name[j][k] == '\0')
            {
              z = (int)(((uint32_t)dat->position[j] - l) >> 2);
              break;
            }
          }
        }
        if(z == -1)
        {
          fprintf(stream2, "%s   %s %s\n", ins, reg, value);
        }
        else
        {
          fprintf(stream2, "%s   %s %d\n", ins, reg, z);
        }
        fscanf(stream1, "%[^\n]", line);
        break;
      }
      else if (c)
      {
        break;
      }
    }
    if(print || c)
    {
      fprintf(stream2, "%s\n", line);
    }
    free(line);
    line = malloc(256);
    for(int i = 0; i < 256; i++)
    {
      line[i] = 0;
    }
    char c = fgetc(stream1);
    while(isspace(c))
    {
      c = fgetc(stream1);
    }
    ungetc(c, stream1);
    fscanf(stream1, "%[^\n]", line );
    l += 0x4;
  }
  free(line);
}

int  strEquals(char* str, char* str2, int length)
{
  for(int i = 0; i < length; i++)
  {
    if(str[i] != str2[i] && str2[i] != '\0')
    {
      return 0;
    }
    else if(str2[i] == '\0')
    {
      break;
    }
  }
  return 1;
}
/**
* Returns 0 to N powers of 10.  Obviously limited to the size of an int.
*/
int power10(int i)
{
  int ret = 1;
  for(int j = 0; j < i; j++)
  {
    ret *= 10;
  }
  return ret;
}

void undoGet(FILE *stream, char *str)
{
  int copy = 0;
  for(int i = 255; i >= 0; i--)
  {
    if(copy && str[i] != '\0')
    {
      ungetc(str[i], stream);
    }
    else if (str[i] == '\0')
    {
      copy = 1;
    }
  }
}

/**
* Print out the symbols I have thus far.
*/
void printData(FILE *stream, Data *dat)
{
  fprintf(stream, "Address     Symbol\n");
  fprintf(stream, "------------------\n");
  for(int i = 0; i < dat->size; i++)
  {
    if(dat->Name[i] != NULL)
    {
      fprintf(stream, "0x%08X  %s\n", dat->position[i], dat->Name[i]);
    }
  }
}

/**
*  Now that I have an input file I can use, I want to translate data and stuff.
*/
Data* step2(FILE *stream1, FILE *stream2)
{
  char *line = malloc(256);
  for(int i = 0; i < 256; i++)
  {
    line[i] = 0;
  }
  int address = 0x2000;
  fscanf(stream1, "%[^\n]", line );
  int count = 0;
  int dataC = 0;
  int c = 0;
  int tAdd = 0x0;
  while(!feof(stream1))
  {
    if(line[0] == '.' && line[1] == 'd' && line[2] == 'a' && line[3] == 't' && line[4] == 'a')
    {
      c = 1;
    }
    for(int i = 0; i < 256; i++)
    {
      if(isspace(line[i]) || line[i] == '\0')
      {
        break;
      }
      else if (line[i] == ':')
      {
        if(c)
        {
          dataC++;
        }
        count++;
        break;
      }
    }
    free(line);
    line = malloc(256);
    for(int i = 0; i < 256; i++)
    {
      line[i] = 0;
    }
    char c = fgetc(stream1);
    while(isspace(c))
    {
      c = fgetc(stream1);
    }
    ungetc(c, stream1);
    fscanf(stream1, "%[^\n]", line );
  }
  free(line);
  line = malloc(256);
  for(int i = 0; i < 256; i++)
  {
    line[i] = 0;
  }
  Data *ret = malloc(sizeof(Data));
  ret->size = count;
  ret->numOfData = dataC;
  ret->Name = malloc(count * sizeof(char*));
  ret->position = malloc(count * sizeof(int));
  /**
  * I've established the number of symbols I need.  Lets make them.
  */
  rewind(stream1);
  for(int i = 0; i < 256; i++)
  {
    line[i] = 0;
  }
  fscanf(stream1, "%[^\n]", line );
  int pos = 0;
  c = 0;
  while(!feof(stream1))
  {
    if(line[0] == '.' && line[1] == 'd' && line[2] == 'a' && line[3] == 't' && line[4] == 'a')
    {
      c = 1;
    }
    int print = 1;
    for(int i = 0; i < 256; i++)
    {
      if (line[i] == ':' && !c)
      {
        print = 0;
        ret->Name[pos] = malloc(i + 1);
        ret->Name[pos][i] = '\0';
        for(int j = i - 1; j >= 0; j--)
        {
          ret->Name[pos][j] = line[j];
        }
        ret->position[pos] = tAdd;
        pos++;
        break;
      }
      else if (line[i] == 'l' && line[i+1] == 'a' && !c)
      {
        char reg[5];
        char in[3];
        char label[50];
        undoGet(stream1, line);
        fscanf(stream1, "%s%s%s", in, reg, label);
        print = 0;
        tAdd += 0x4;
        fprintf(stream2, "addi %s $zero, %s\n", reg, label);
        char tempLine[256];
        fscanf(stream1, "%[^\n]", tempLine );
        break;
      }
      else if (line[i] == 'l' && line[i+1] == 'i' && !c)
      {
        char reg[5];
        char in[3];
        int im;
        undoGet(stream1, line);
        fscanf(stream1, "%s%s%d", in, reg, &im);
        print = 0;
        tAdd += 0x4;
        if(im > 0xFFFF)
        {
          tAdd += 0x4;
          fprintf(stream2, "lui   %s %X\nori %s %d\n", reg, (im >> 16) & 0xFFFF, reg, im && 0xFFFF);
        }
        else
        {
          fprintf(stream2, "addiu %s $zero, %d\n", reg, im & 0xFFFF);
        }
        char tempLine[256];
        fscanf(stream1, "%[^\n]", tempLine );
        break;
      }
      else if (line[i] == 'b' && line[i+1] == 'l' && line[i+2] == 't' && line[i+3] == ' ' && !c)
      {
        char in[4];
        char rs[5];
        char rt[5];
        int im;
        print = 0;
        undoGet(stream1, line);
        fscanf(stream1, "%s%s%s%d", in, rs, rt, &im);
        rt[4] = '\0';
        tAdd += 0x8;
        fprintf(stream2, "sub   $at, %s %s\nbltz  $at, %d", rs, rt, im);
        char tempLine[256];
        fscanf(stream1, "%[^\n]", tempLine );
        break;
      }
      else if (line[i] == 'n' && line[i+1] == 'o' && line[i+2] == 'p' && line[i+3] == ' ' && !c)
      {
        print = 0;
        fprintf(stream2, "sll   $zero, $zero, 0");
        break;
      }
      else if (line[i] == 'b' && line[i+1] == 'l' && line[i+2] == 'e' && line[i+3] == ' ' && !c)
      {
        char in[4];
        char rs[5];
        char rt[5];
        int im;
        print = 0;
        undoGet(stream1, line);
        fscanf(stream1, "%s%s%s%d", in, rs, rt, &im);
        rt[4] = '\0';
        tAdd += 0x8;
        fprintf(stream2, "sub   $at, %s %s\nblez  $at, %d", rs, rt, im);
        char tempLine[256];
        fscanf(stream1, "%[^\n]", tempLine );
        break;
      }
      else if (line[i] == ':' && c)
      {
        print = 0;
        ret->Name[pos] = malloc((i + 1) * sizeof(char));
        ret->Name[pos][i] = '\0';
        for(int j = i - 1; j >= 0; j--)
        {
          ret->Name[pos][j] = line[j];
        }
        ret->position[pos] = address;
        undoGet(stream1, line);
        char name[i];
        char type[8];
        fscanf(stream1, "%s%s", name, type);
        int add = 0;
        if(strEquals(type, ".word", 8))
        {
          char length[14];
          fscanf(stream1, "%s", length);
          for(int j = 0; j < 14; j++)
          {
            if(length[j] == ':')
            {
              j++;
              int k = 0;
              int value = 0;
              while (1)
              {
                 if(length[j+k] =='\0')
                 {
                   break;
                 }
                 k++;
              }
              int z = 0;
              while(1)
              {
                if (length[j+k] - 48 < 0)
                {
                  k--;
                  continue;
                }
                value += (length[j + k] - 48) * power10(z) * 0x4;
                z++;
                k--;
                if(k == -1)
                {
                  break;
                }
              }
              add = value;
              break;
            }
            else if(length[j] == '\0')
            {
              add = 0x4;
              break;
            }
          }
        }
        else
        {
          char tempLine[256];
          fscanf(stream1, "%[^\n]", tempLine );
          int j = 0;
          while(tempLine[j] != '"')
          {
            j++;
          }
          j++;
          while(tempLine[j] != '"')
          {
            add += 0x4;
            j++;
          }
          add += 0x4;
        }
        address += add;
        pos++;
        break;
      }
    }
    if(c || print)
    {
      if(!c && line[0] != '.')
      {
        tAdd += 0x4;
      }
      fprintf(stream2, "%s\n", line);
    }
    free(line);
    line = malloc(256);
    for(int i = 0; i < 256; i++)
    {
      line[i] = 0;
    }
    char c = fgetc(stream1);
    while(isspace(c))
    {
      c = fgetc(stream1);
    }
    ungetc(c, stream1);
    fscanf(stream1, "%[^\n]", line );
    c = 0;
  }
  free(line);
  return ret;
}


/**
* I need to simplify the problem by removing lines that are comments and
* the blank lines.
*/
void simplify(FILE *stream1, FILE *stream2)
{
  char *line = malloc(256);
  int override = 0;
  int hasContent = 0;
  int ignore = 0;
  for(int i = 0; i < 256; i++)
  {
    line[i] = 0;
  }
  fscanf(stream1, "%[^\n]", line );
  while(!feof(stream1))
  {
    for(int i = 0; i < 256; i++)
    {
      if(line[i] == '"')
      {
        ignore = !ignore;
        hasContent = 1;
      }
      if(override && !ignore)
      {
        line[i] = '\0';
        break;
      }
      else if (!ignore)
      {
        if(line[i] == '#')
        {
          override = 1;
          line[i] = '\0';
        }
        else if (!isspace(line[i]) && line[i] != '\0')
        {
          hasContent = 1;
        }
      }
    }
    if (hasContent)
    {
      hasContent = !hasContent;
      fprintf(stream2, "%s\n", line);
    }
    free(line);
    line = malloc(256);
    for(int i = 0; i < 256; i++)
    {
      line[i] = 0;
    }
    char c = fgetc(stream1);
    while(isspace(c))
    {
      c = fgetc(stream1);
    }
    ungetc(c, stream1);
    fscanf(stream1, "%[^\n]", line );
    override = 0;
    ignore = 0;
  }
  free(line);
}




//Print R type to file.
void print_R(FILE *stream, const int rs, const int rt, const int rd, const int shamt, const int fun)
{
  char *rCode = convertB(OpCode[0].Value, 6);
  char *prs = convertB(rs, 5);
  char *prt = convertB(rt, 5);
  char *prd = convertB(rd, 5);
  char *pshamt = convertB(shamt, 5);
  char *pfun = convertB(fun, 6);
  fprintf(stream, "%s%s%s%s%s%s\n", rCode, prs, prt, prd, pshamt, pfun);
  free(rCode);
  free(prs);
  free(prt);
  free(prd);
  free(pshamt);
  free(pfun);
}



//Print I type to file
void print_I(FILE *stream, const int op, const int rs, const int rt, const int imm)
{
  char *rCode = convertB(op, 6);
  char *prs = convertB(rs, 5);
  char *prt = convertB(rt, 5);
  char *pimm = convertB(imm, 16);
  fprintf(stream, "%s%s%s%s\n", rCode, prs, prt, pimm);
  free(rCode);
  free(prs);
  free(prt);
  free(pimm);
}



//Print J type to file
void print_J(FILE *stream, const int op, const int imm)
{
  char *rCode = convertB(op, 6);
  char *pimm = convertB(imm, 26);
  fprintf(stream, "%s%s\n", rCode, pimm);
  free(rCode);
  free(pimm);
}



//Convert number to a fixed size char array.
//Remember to free array after use.
char* convertB(const int i, const int size)
{
  char *ret = malloc((size + 1) * sizeof(char));
  int workingI = i;
  for(int j = 1; j <= size; j++)
  {
    ret[size - j] = (workingI & 1) + 48;
    workingI >>= 1;
  }
  ret[size] = '\0';
  return ret;
}
