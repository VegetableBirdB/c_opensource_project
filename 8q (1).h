#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<stdbool.h>
#include<string.h>
#include<ctype.h>

#define MAX_SIZE 15
#define MAX_STRARR_SIZE 1000000
#define LOWER_SIZE 0
#define UPPER_SIZE 10
#define SPACE '#'
#define QUEEN 'q'
typedef struct{
  char grid[MAX_SIZE][MAX_SIZE];
  int size;
  int q_cnt;
  bool is_q_in_rows[MAX_SIZE];
  bool is_q_in_cols[MAX_SIZE];  
}Qmap;

bool check_input(int argc,char* argv[]);  
Qmap init_qmap(int size);
int solve_qmap(Qmap qmaps[],int* rear_addr);
void print_ans(bool is_output_map,Qmap qmaps[],
                             int solution_num,
                             int rear);
int add_qmaps(Qmap qmaps[],Qmap parent, int rear); 
void test(void);
bool is_insert_queen(int row,int col,Qmap parent);
bool is_exist_qmap(Qmap qmaps[], Qmap child, int rear); 

