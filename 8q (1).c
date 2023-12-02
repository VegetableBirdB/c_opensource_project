#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<assert.h>
#include<pthread.h>

#define QUEEN 'Q'
#define SPACE ' '
#define MAX_SIZE 30
#define UPPER_BOUND 10
#define LOWER_BOUND 0
#define PARAM_ONE 1
#define PARAM_TWO 2
#define PARAM_THREE 3
#define BLACK 30
#define WHITE 97
#define BLUE 104
#define CYAN 106
/*I am so sorry to use local variable. Because with the purpose of 
achieving synchronous operation, I can't think of any methods
except using mutex*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
  long long col_limit;
  long long angle_45_limit;
  long long angle_135_limit;
  long long upper_limit;
  long long permute[MAX_SIZE];
  long long cur_row;
  int size;
  long long convert[1<<(MAX_SIZE-1)];
  long long cnt_solu;
  bool is_output_map;
  bool flag;
}Func_param;

void print_permute(long long permute[],int size);
void draw_permute(long long permute[],int size);
bool check_input(int argc,char* argv[]);
void test();
void run_pthread(int size, bool is_output_map);
void* func_threads(void* param);
void dfs_mutiple_thread(long long col_limit, long long angle_45_limit, long long angle_135_limit, 
long long upper_limit, long long permute[], long long *cur_row, int size, 
long long convert[], long long*cnt_solu, bool is_output_map, bool flag);
void init_func_params(Func_param*func_params[],int end);
void init_param_func_in_threads(long long param_func_in_threads[][3],
                          long long possible_pos_in_row1[],int end);
void set_func_params_convert(Func_param* func_params[],int end); 
void set_func_params_other_elements(Func_param* func_params[],
int end,bool is_odd,bool is_output_map,long long param_func_in_threads[][3],
long long possible_pos_in_row1[],int size);
void pthreads_create(pthread_t threads[], int end, Func_param* func_params[]);
void pthreads_join(pthread_t threads[], int end); 
void print_pthreads_solutions(Func_param* func_params[],int end,int size);
void free_func_params(Func_param* func_params[], int end);
void calc_possible_pos_in_row1(long long possible_pos, long long possible_pos_in_row1[],
                int index,long long possible_pos_odd,bool is_odd);

int main(int argc, char* argv[]) 
{
  //test();
  int size;
  bool is_output_map = check_input(argc,argv);
  if(is_output_map){
    sscanf(argv[PARAM_TWO],"%i",&size);
  }else{
    sscanf(argv[PARAM_ONE],"%i",&size);
  } 
  run_pthread(size,is_output_map);
  return 0;
}

bool check_input(int argc,char* argv[])
{ 
  int num;
  if(argc==PARAM_TWO)
  {
    if(sscanf(argv[PARAM_ONE],"%d",&num)!=1||num<=LOWER_BOUND||num>MAX_SIZE){
      fprintf(stderr,"wrong input of one input");
      exit(EXIT_FAILURE);
    }
    return false;  
  }else if(argc==PARAM_THREE){
    if(strcmp(argv[PARAM_ONE],"-verbose")!=0||
        sscanf(argv[PARAM_TWO],"%d",&num)!=1||
                               num<=LOWER_BOUND||
                               num>MAX_SIZE){
      fprintf(stderr,"wrong input of two inputs");           
      exit(EXIT_FAILURE); 
    }
    return true;
  }else{
    fprintf(stderr,"wrong number of inputs");           
    exit(EXIT_FAILURE);
  }
}

//print available permutations 
void print_permute(long long permute[],int size)
{
  for(int i = 1;i <= size;++i){
    if(permute[i]>=UPPER_BOUND){
      printf("%c",(int)(permute[i]-UPPER_BOUND+'A'));
    }
    else{
      printf("%lld",permute[i]);
    }
  }
  printf("\n");
  //in terms of principle of symmetry, print another permutation.
  for(int i = 1;i <= size;++i){
    if(size+1-permute[i]>=UPPER_BOUND){
      printf("%c",(int)(size+1-permute[i]-UPPER_BOUND+'A'));
    }
    else{
      printf("%lld",size+1-permute[i]);
    } 
  }
  printf("\n");
}

void dfs_mutiple_thread(long long col_limit, long long angle_45_limit, long long angle_135_limit, 
long long upper_limit, long long permute[], long long *cur_row, int size, 
long long convert[], long long *cnt_solu, bool is_output_map, bool flag)
{
  if(col_limit == upper_limit){
    ++*cnt_solu;
    if(is_output_map){
      pthread_mutex_lock(&mutex);
      draw_permute(permute,size);
      pthread_mutex_unlock(&mutex);
      printf("\n\n");
    }
  }
  long long possible_pos = upper_limit & (~(col_limit | 
                        angle_45_limit | angle_135_limit));
  if(*cur_row==1&&flag){
    possible_pos=possible_pos&((1<<(size>>1))-1);
  }
  while(possible_pos){
    long long right_first_bit_not_zero=possible_pos&
                                           -possible_pos;
    possible_pos^=right_first_bit_not_zero;
    permute[++*cur_row]=convert[right_first_bit_not_zero];
    //start to do recursion
    dfs_mutiple_thread(col_limit|right_first_bit_not_zero,
    (angle_45_limit|right_first_bit_not_zero)<<1,
    (angle_135_limit|right_first_bit_not_zero)>>1, upper_limit, 
    permute, cur_row, size, convert, cnt_solu, is_output_map, flag);
    //start to do backtrace
    --*cur_row;
  }
}

//function which correspond to one thread
void* func_threads(void* param)
{
  Func_param* f = (Func_param*)param;
  dfs_mutiple_thread(f->col_limit, f->angle_45_limit, f->angle_135_limit, f->upper_limit, f->permute, &f->cur_row , f->size, 
                    f->convert,&f->cnt_solu, f->is_output_map, f->flag);
  return NULL;
}

void init_func_params(Func_param*func_params[],int end)
{
  for(int i=0;i<end;i++){
    func_params[i] = (Func_param*)calloc(1,sizeof(Func_param));
  }
} 

void init_param_func_in_threads(long long param_func_in_threads[][3],long long possible_pos_in_row1[],int end)
{
  for(register int i=0;i<end;i++){
    param_func_in_threads[i][0] = possible_pos_in_row1[i];
    param_func_in_threads[i][1] = possible_pos_in_row1[i]<<1; 
    param_func_in_threads[i][2] = possible_pos_in_row1[i]>>1;
  }
}

void set_func_params_convert(Func_param* func_params[],int end)
{
  for(register int j=0;j<end;j++){
    func_params[j]->convert[1] = 1;
    for(register int i = 0;i < MAX_SIZE-1;++i){
      func_params[j]->convert[1<<i]=i+1;
    }
  }
}

void set_func_params_other_elements(Func_param* func_params[],
int end,bool is_odd,bool is_output_map,long long param_func_in_threads[][3],
long long possible_pos_in_row1[],int size)
{
  for(register int j=0;j<end;j++){
    if(j==end-1&&is_odd){
      func_params[j]->flag = true;
    }
    func_params[j]->permute[1] = func_params[j]->convert[possible_pos_in_row1[j]];
    func_params[j]->upper_limit = (1 << size) - 1;
    func_params[j]->size = size;
    func_params[j]->cur_row = 1;
    func_params[j]->is_output_map = is_output_map;
    func_params[j]->col_limit = param_func_in_threads[j][0];
    func_params[j]->angle_45_limit = param_func_in_threads[j][1];
    func_params[j]->angle_135_limit = param_func_in_threads[j][2]; 
  }
}
void pthreads_create(pthread_t threads[], int end, Func_param* func_params[])
{
  for(register int i=0;i<end;i++){
    pthread_create(&threads[i],NULL,func_threads,(void*)func_params[i]);
  }
}

void pthreads_join(pthread_t threads[], int end)
{
  for(register int i=0;i<end;i++){
    pthread_join(threads[i],NULL);
  }
}

void print_pthreads_solutions(Func_param* func_params[],int end,int size)
{
  long long sum_solutions = 0;
  for(register int i=0;i<end;i++){
    sum_solutions+=(func_params[i]->cnt_solu<<1);
  }
  if(size==1){
    printf("%lld solutions", sum_solutions>>1); 
  }else{
    printf("%lld solutions", sum_solutions);
  }
}

void free_func_params(Func_param* func_params[], int end)
{
  for(register int i=0;i<end;i++){
    free(func_params[i]);
  }
}

void calc_possible_pos_in_row1(long long possible_pos, long long possible_pos_in_row1[],
                int index,long long possible_pos_odd,bool is_odd)
{
  while(possible_pos){
    long long right_first_bit_not_zero=possible_pos&
                                           -possible_pos;
    possible_pos_in_row1[index++] = right_first_bit_not_zero;
    possible_pos^=right_first_bit_not_zero;
  }
  if(is_odd){
    possible_pos_in_row1[index++] = possible_pos_odd;
  }
}

//start to solve n queens problem with multiple threadings
void run_pthread(int size, bool is_output_map)
{
  long long possible_pos_odd;
  bool is_odd = false;
  if(size&1){
    possible_pos_odd = 1<<(size>>1);
    is_odd = true;
  }
  int end = is_odd?(size>>1)+1:(size>>1);
  pthread_t threads[MAX_SIZE>>1];
  Func_param* func_params[MAX_SIZE>>1];
  init_func_params(func_params,end);
  long long possible_pos = (1<<(size>>1))-1,index = 0;
  long long possible_pos_in_row1[MAX_SIZE>>1]; 
  calc_possible_pos_in_row1(possible_pos, possible_pos_in_row1,
                            index, possible_pos_odd, is_odd);
  long long param_func_in_threads[MAX_SIZE>>1][3];
  init_param_func_in_threads(param_func_in_threads,possible_pos_in_row1,end);
  set_func_params_convert(func_params, end);

  set_func_params_other_elements(func_params, end, is_odd, 
  is_output_map, param_func_in_threads, possible_pos_in_row1, size);

  pthreads_create(threads, end, func_params); 
  pthreads_join(threads, end);
  print_pthreads_solutions(func_params, end, size);
  free_func_params(func_params, end); 
  pthread_mutex_destroy(&mutex);
}

// print board
void draw_permute(long long permute[],int size)
{ 
  for(int i=0;i<size;i++){
    if(size-i>=UPPER_BOUND){
      printf("\033[%dm%c\033[m",WHITE,'A'+ size-i-UPPER_BOUND); 
    }else{
      printf("\033[%dm%d\033[m",WHITE,size-i); 
    }
    for(int j=0;j<size;j++){
      if(j+1==permute[i+1]){
        if((i%2!=0&&j%2!=0)||(i%2==0&&j%2==0)){
          printf("\033[%dm\033[%dm%2c\033[m",BLUE,BLACK,QUEEN); 
        }else{
          printf("\033[%dm\033[%dm%2c\033[m",CYAN,BLACK,QUEEN);
        }
      }
      else if((i%2!=0&&j%2!=0)||(i%2==0&&j%2==0)){
        printf("\033[%dm%2c\033[m",BLUE,SPACE); 
      }else{
        printf("\033[%dm%2c\033[m",CYAN,SPACE);
      }
    }
    printf("\n");
    if(i==size-1){
      for(int k=1;k<=size;k++){
        if(k>=UPPER_BOUND){
          printf("\033[%dm %c\033[m",WHITE,k-UPPER_BOUND+'A');
        }else{
          printf("\033[%dm %d\033[m",WHITE,k); 
        }
      }
      printf("\n");
    }
  }
  if(size==1){
    return;
  }
  printf("\n\n");
  for(int i=0;i<size;i++){
    if(size-i>=10){
      printf("\033[%dm%c\033[m",WHITE,(int)(size+'A'-i-UPPER_BOUND));
    }else{
      printf("\033[%dm%d\033[m",WHITE,size-i);
    }
    for(int j=0;j<size;j++){
      if(j+1==size+1-permute[i+1]){
        if((i%2!=0&&j%2!=0)||(i%2==0&&j%2==0)){
          printf("\033[%dm\033[%dm%2c\033[m",BLUE,BLACK,QUEEN);
        }else{
          printf("\033[%dm\033[%dm%2c\033[m",CYAN,BLACK,QUEEN);
        }
      }
      else if((i%2!=0&&j%2!=0)||(i%2==0&&j%2==0)){
        printf("\033[%dm%2c\033[m",BLUE,SPACE);
      }else{
        printf("\033[%dm%2c\033[m",CYAN,SPACE);
      }
    }
    printf("\n");
    if(i==size-1){
      for(int k=1;k<=size;k++){
        if(k>=UPPER_BOUND){
          printf("\033[%dm %c\033[m",WHITE,k-10+'A');
        }else{
          printf("\033[%dm %d\033[m",WHITE,k);
        }
      }
      printf("\n");
    }
  }
}
void test()
{ 
  //print_permute no need to test
  //check_input
  char* argv0[2] = {" ","4"};
  char* argv1[3] = {" ","-verbose","5"};
  assert(check_input(2,argv0)==false);
  assert(check_input(3,argv1)==true);
  
  //test dfs
  /*
  int size = 10;
  int convert[1 << (MAX_SIZE-1)];
  convert[1] = 1;
  int flag = 1, upper_limit = (1 << size) - 1;
  int permute[MAX_SIZE] = {0};
  int cur_row = 0;
  int cnt_solu = 0;
  for(register int i = 0;i < MAX_SIZE-1;++i){
    convert[1<<i]=i+1;
  }
  dfs(0, 0, 0, upper_limit, permute, &cur_row, flag, size, 
                    convert,&cnt_solu, 1);
  //printf("%d",cnt_solu);
  assert((cnt_solu<<1) == 724);*/

  //test dfs
  /*
  int size = 16;
  int convert[1 << (MAX_SIZE-1)];
  convert[1] = 1;
  int flag = 1, upper_limit = (1 << size) - 1;
  int permute[MAX_SIZE] = {0};
  int cur_row = 0;
  int cnt_solu = 0;
  for(register int i = 0;i < MAX_SIZE-1;++i){
    convert[1<<i]=i+1;
  }
  dfs(0, 0, 0, upper_limit, permute, &cur_row, flag, size, 
                    convert,&cnt_solu, 1);
  assert((cnt_solu<<1) == 14772512);
  */
}
