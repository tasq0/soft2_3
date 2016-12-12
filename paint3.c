#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>

#define WIDTH 70
#define HEIGHT 40

#define BUFSIZE 1000

char canvas[WIDTH][HEIGHT];

//////////////////////////////////////////////////////////
// リスト操作
//////////////////////////////////////////////////////////

struct node 
{
  char *str;
  struct node *next;
};

typedef struct node Node;

Node *push_front(Node *begin, const char *str)
{
  // Create a new element
  Node *p = malloc(sizeof(Node));
  char *s = malloc(strlen(str) + 1);
  strcpy(s, str);
  p->str = s;
  p->next = begin; 

  return p;  // Now the new element is the first element in the list
}

Node *pop_front(Node *begin)
{
  assert(begin != NULL); // Don't call pop_front() when the list is empty
  Node *p = begin->next;

  free(begin->str);
  free(begin);

  return p;
}

Node *push_back(Node *begin, const char *str)
{
  if (begin == NULL) {   // If the list is empty
    return push_front(begin, str);
  }

  // Find the last element
  Node *p = begin;
  while (p->next != NULL) {
    p = p->next;
  }

  // Create a new element
  Node *q = malloc(sizeof(Node));
  char *s = malloc(strlen(str) + 1);
  strcpy(s, str);
  q->str = s;
  q->next = NULL;

  // The new element should be linked from the previous last element
  p->next = q;

  return begin;
}

Node *remove_all(Node *begin)
{
  while ((begin = pop_front(begin))) 
    ; // Repeat pop_front() until the list becomes empty
  return begin;  // Now, begin is NULL
}

Node *pop_back(Node *begin)
{
  Node *p = begin;
  //要素が1または0ならnullを返す
  if(p == NULL){
    return NULL;
  }
  if(p->next == NULL){
    return NULL;
  }
  while(p->next->next != NULL){
    p = p->next;
  }
  
  free(p->next);
  free(p->next->str);
  
  p->next = NULL;
  
  return begin;
}

////////////////////////////////////////////
// リスト操作ここまで
////////////////////////////////////////////

const char *default_history_file = "history.txt";
const char *default_picture_file = "pic.txt";
int numofanimation = 0;
int numofmovingthingswhenanimationfinished = 1;

int interpret_command(const char *, int *, Node **, FILE *);
int deleteanimationfile();

void print_canvas(FILE *fp)
{
  int x, y;

  fprintf(fp, "----------\n");

  for (y = 0; y < HEIGHT; y++) {
    for (x = 0; x < WIDTH; x++) {
      const char c = canvas[x][y];
      fputc(c, fp);
    }
    fputc('\n', fp);
  }
  fflush(fp);
}

void init_canvas()
{
  memset(canvas, ' ', sizeof(canvas));
}

int max(const int a, const int b)
{
  return (a > b) ? a : b;
}

void draw_line(const int x0, const int y0, const int x1, const int y1)
{
  int i;
  const int n = max(abs(x1 - x0), abs(y1 - y0));
  for (i = 0; i <= n; i++) {
    const int x = x0 + i * (x1 - x0) / n;
    const int y = y0 + i * (y1 - y0) / n;
    canvas[x][y] = '#';
  }
}

//
//左上の頂点の座標と横及び縦の長さを入力すると長方形を描きます
void draw_rectangle(const int x0, const int y0, const int width, const int height)
{
  draw_line(x0, y0, x0+width-1, y0);
  draw_line(x0, y0, x0, y0+height-1);
  draw_line(x0+width-1, y0, x0+width-1, y0+height-1);
  draw_line(x0, y0+height-1, x0+width-1, y0+height-1);
}

//
//中心の座標と半径の長さを入力すると円を描きます
void draw_circle(x0, y0, r)
{
  const int n = trunc(2 * 3.14 * r);
  int i;

  for(i = 0; i <= n; i++){
    const int x = trunc(x0 + r * cos(2 * 3.14 * i / n));
    const int y = trunc(y0 + r * sin(2 * 3.14 * i / n));
    if(x<WIDTH && x>0 && y<HEIGHT && y>0){
      canvas[x][y] = '#';
    }
  }
}

void save_history(const char *filename, Node *begin)
{
  if (filename == NULL)
    filename = default_history_file;
  
  FILE *fp;
  if ((fp = fopen(filename, "w")) == NULL) {
    fprintf(stderr, "error: cannot open %s.\n", filename);
    return;
  }
  
  Node *p = begin;
  while(p!=NULL){
    fprintf(fp, "%s", p -> str);
    p = p->next;
  }

  printf("saved as \"%s\"\n", filename);

  fclose(fp);
}

Node *load_history(const char *filename, Node *begin, int *hsize)
{
  char buf[BUFSIZE];
  int hsize0 = 0;
  
  if (filename == NULL)
    filename = default_history_file;

  FILE *fp;
  if((fp = fopen(filename, "r")) == NULL){
    fprintf(stderr, "error: cannot open %s.\n", filename);
    return begin;
  }

  if(begin != NULL){
    begin = remove_all(begin);
  }
  while(fgets(buf, BUFSIZE, fp) != NULL){
    begin = push_back(begin, buf);
    hsize0++;
  }
  *hsize = hsize0;

  printf("loaded from \"%s\"\n", filename);

  fclose(fp);

  return begin;
}

char **load_pic(int *mx, int *my, const char *filename)
{
  int i, j, x=1, x_max=1, y=1;
  char **s;
  char c;

  if(filename == NULL)
    filename = default_picture_file;

  FILE *fp;
  if((fp = fopen(filename, "r")) == NULL){
    fprintf(stderr, "error: cannot open %s.\n", filename);
    return NULL;
  }

  while((c = fgetc(fp)) != EOF){
    if(c !='\n'){
      x++;
      if(x>x_max){
	x_max = x;
      }
    }
    else{
      x = 1;
      y++;
    }
  }

  s = (char **)malloc(sizeof(char *) * y);
  for(i=0; i<y; i++){
    s[i] = (char *)malloc(sizeof(char *) * x_max);
    memset(s[i], '\0', x_max);
  }
  
  fseek(fp, 0, SEEK_SET);

  for(i=0; i<y; i++){
    for(j=0; j<x_max; j++){
      c = fgetc(fp);
      if(c == '\n'||c == EOF){
	break;
      }
      s[i][j] = c;
    }
  }

  fclose(fp);

  *mx = x_max;
  *my = y;
  return s;
}

void draw_picture(const int x, const int y, const char *filename)
{
  int mx,my,i,j;
  char **pic;
  pic = load_pic(&mx, &my, filename);

  for(i = 0; i<my; i++){
    for(j = 0; j<mx; j++){
      if(pic[i][j] =='\0'){
	break;
      }
      else{
	if(x+j<WIDTH && x+j>0 && y+i<HEIGHT && y+i>0){
	  //xとyが逆らしいけど面倒なのでここだけ直す
	  canvas[x+j][y+i] = pic[i][j];
	}
      }
    }
  }
}

void animation_line(Node *begin, const int x0, const int y0, const int x1, const int y1, const char *filename)
{
  int i;
  const int n = max(abs(x1 - x0), abs(y1 - y0));
  
  for (i = 0; i <= n; i++) {
    char filename_ani[32];
    FILE *fp;
    
    sprintf(filename_ani,"animation/animation%d.txt",numofanimation);
    numofanimation++;
    
    const int x = x0 + i * (x1 - x0) / n;
    const int y = y0 + i * (y1 - y0) / n;

    save_history(filename_ani, begin);

    fp = fopen(filename_ani, "a");
    if(filename == NULL){
      fprintf(fp,"pic %d %d\n", x, y);
    }
    else{
      fprintf(fp,"pic %d %d %s\n", x, y, filename);
    }
    fclose(fp);
  }
}

void animation_circle(Node *begin, const int x0, const int y0, const int r, const char *filename)
{
  const int n = trunc(2 * 3.14 * r);
  int i;
  
  for (i = 0; i <= n; i++) {
    char filename_ani[32];
    FILE *fp;
    
    sprintf(filename_ani,"animation/animation%d.txt",numofanimation);
    numofanimation++;
    
    const int x = trunc(x0 + r * cos(2 * 3.14 * i / n));
    const int y = trunc(y0 + r * sin(2 * 3.14 * i / n));

    save_history(filename_ani, begin);

    fp = fopen(filename_ani, "a");
    if(filename == NULL){
      fprintf(fp,"pic %d %d\n", x, y);
    }
    else{
      fprintf(fp,"pic %d %d %s\n", x, y, filename);
    }
    fclose(fp);
  }
}

void animation_sin(Node *begin, const int x0, const int y0, const int A, const int dist, const char *filename)
{
  int y;
  if(dist > 0){
    for(y = y0; y<=y0+dist; y++){
      char filename_ani[32];
      FILE *fp;
    
      sprintf(filename_ani,"animation/animation%d.txt",numofanimation);
      numofanimation++;
    
      const int x = trunc(x0 + A * sin(y-y0));

      save_history(filename_ani, begin);

      fp = fopen(filename_ani, "a");
      if(filename == NULL){
	fprintf(fp,"pic %d %d\n", x, y);
      }
      else{
	fprintf(fp,"pic %d %d %s\n", x, y, filename);
      }
      fclose(fp);
    }
  }
  if(dist < 0){
    for(y = y0; y>=y0+dist; y--){
      char filename_ani[32];
      FILE *fp;
    
      sprintf(filename_ani,"animation/animation%d.txt",numofanimation);
      numofanimation++;
    
      const int x = trunc(x0 + A * sin(y-y0));

      save_history(filename_ani, begin);

      fp = fopen(filename_ani, "a");
      if(filename == NULL){
	fprintf(fp,"pic %d %d\n", x, y);
      }
      else{
	fprintf(fp,"pic %d %d %s\n", x, y, filename);
      }
      fclose(fp);
    }
  }
}

void plus_animation_line(Node *begin, const int x0, const int y0, const int x1, const int y1, const int step0, const char *filename)
{
  int i;
  int step = step0;
  const int n = max(abs(x1 - x0), abs(y1 - y0));
  
  for(i =0;i<=n;i++){
    char filename_ani[32];
    FILE *fp;
    
    if(step >= numofanimation){
      numofmovingthingswhenanimationfinished ++;
      break;
    }
    
    sprintf(filename_ani,"animation/animation%d.txt", step);
    step++;
    
    const int x = x0 + i * (x1 - x0) / n;
    const int y = y0 + i * (y1 - y0) / n;

    fp = fopen(filename_ani, "a");
    if(filename == NULL){
      fprintf(fp,"pic %d %d\n", x, y);
    }
    else{
      fprintf(fp,"pic %d %d %s\n", x, y, filename);
    }
    fclose(fp);
  }
}

void plus_animation_circle(Node *begin, const int x0, const int y0, const int r, const int step0, const char *filename)
{
  int i;
  int step = step0;
  const int n = trunc(2 * 3.14 * r);
  
  for(i =0;i<=n;i++){
    char filename_ani[32];
    FILE *fp;
    
    if(step >= numofanimation){
      numofmovingthingswhenanimationfinished ++;
      break;
    }
    
    sprintf(filename_ani,"animation/animation%d.txt", step);
    step++;
    
    const int x = trunc(x0 + r * cos(2 * 3.14 * i / n));
    const int y = trunc(y0 + r * sin(2 * 3.14 * i / n));

    fp = fopen(filename_ani, "a");
    if(filename == NULL){
      fprintf(fp,"pic %d %d\n", x, y);
    }
    else{
      fprintf(fp,"pic %d %d %s\n", x, y, filename);
    }
    fclose(fp);
  }
}


void animation_play(int *hsize_p, Node **begin_p, FILE *fp)
{
  char buf[BUFSIZE];
  int i;
  for(i=0; i<numofanimation; i++){
    sprintf(buf,"load animation/animation%d.txt\n", i);
    interpret_command(buf, hsize_p, begin_p, NULL);
    print_canvas(fp);
    init_canvas();
    usleep(50 * 1000);
  }
  printf("%d\n",numofmovingthingswhenanimationfinished);
  for (i=0; i<numofmovingthingswhenanimationfinished; i++){
    interpret_command("undo\n", hsize_p, begin_p, NULL);
  }
}

// Interpret and execute a command
//   return value:
//     0, normal commands such as "line"
//     1, unknown or special commands (not recorded in history[])
//     2, quit
int interpret_command(const char *command, int *hsize, Node **begin_p, FILE *fp)
{
  char buf[BUFSIZE];
  strcpy(buf, command);
  buf[strlen(buf) - 1] = 0; // remove the newline character at the end

  const char *s = strtok(buf, " ");

  if (strcmp(s, "line") == 0) {
    int x0, y0, x1, y1;
    x0 = atoi(strtok(NULL, " "));
    y0 = atoi(strtok(NULL, " "));
    x1 = atoi(strtok(NULL, " "));
    y1 = atoi(strtok(NULL, " "));
    draw_line(x0, y0, x1, y1);
    return 0;
  }

  if(strcmp(s, "rect") == 0) {
    int x0, y0, width, height;
    x0 = atoi(strtok(NULL, " "));
    y0 = atoi(strtok(NULL, " "));
    width = atoi(strtok(NULL, " "));
    height = atoi(strtok(NULL, " "));
    draw_rectangle(x0, y0, width, height);
    return 0;
  }

  if(strcmp(s, "cir") == 0) {
    int x0, y0, r;
    x0 = atoi(strtok(NULL, " "));
    y0 = atoi(strtok(NULL, " "));
    r = atoi(strtok(NULL, " "));
    draw_circle(x0, y0, r);
    return 0;
  }

  if(strcmp(s, "pic") == 0) {
    int x0, y0;
    x0 = atoi(strtok(NULL, " "));
    y0 = atoi(strtok(NULL, " "));
    s = strtok(NULL, " ");
    draw_picture(x0, y0, s);
    return 0;
  }

  if(strcmp(s, "aniline") == 0) {
    int x0, y0, x1, y1;
    x0 = atoi(strtok(NULL, " "));
    y0 = atoi(strtok(NULL, " "));
    x1 = atoi(strtok(NULL, " "));
    y1 = atoi(strtok(NULL, " "));
    s = strtok(NULL, " ");
    animation_line(*begin_p, x0, y0, x1, y1, s);
    return 3;
  }

  if(strcmp(s, "anicir") == 0) {
    int x0, y0, r;
    x0 = atoi(strtok(NULL, " "));
    y0 = atoi(strtok(NULL, " "));
    r = atoi(strtok(NULL, " "));
    s = strtok(NULL, " ");
    animation_circle(*begin_p, x0, y0, r, s);
    return 3;
  }

  if(strcmp(s, "anisin") == 0) {
    int x0, y0, A, dist;
    x0 = atoi(strtok(NULL, " "));
    y0 = atoi(strtok(NULL, " "));
    A = atoi(strtok(NULL, " "));
    dist = atoi(strtok(NULL, " "));
    s = strtok(NULL, " ");
    animation_sin(*begin_p, x0, y0, A, dist, s);
    return 3;
  }

  if(strcmp(s, "anilinep") == 0){
    int x0, y0, x1, y1, step0;
    x0 = atoi(strtok(NULL, " "));
    y0 = atoi(strtok(NULL, " "));
    x1 = atoi(strtok(NULL, " "));
    y1 = atoi(strtok(NULL, " "));
    step0 = atoi(strtok(NULL, " "));
    s = strtok(NULL, " ");
    plus_animation_line(*begin_p, x0, y0, x1, y1, step0, s);
    return 4;
  }

    if(strcmp(s, "anicirp") == 0) {
      int x0, y0, r, step0;
    x0 = atoi(strtok(NULL, " "));
    y0 = atoi(strtok(NULL, " "));
    r = atoi(strtok(NULL, " "));
    step0 = atoi(strtok(NULL, " "));
    s = strtok(NULL, " ");
    plus_animation_circle(*begin_p, x0, y0, r, step0, s);
    return 4;
  }

  if(strcmp(s, "animation") == 0) {
    animation_play(hsize, begin_p, fp);
    return 1;
  }

  if(strcmp(s, "deleteanimation") == 0) {
    char c;
    printf("すべてのアニメーションを削除しますが、よろしいですか？ y/n\n");
    while(1){
      if((c = getchar()) == 'y'){
	getchar();
	if(deleteanimationfile() == 0){
	  printf("アニメーションの削除が完了しました\n");
	}
	else{
	  printf("アニメーションの削除に失敗しました\n");
	}
	numofanimation = 0;
	numofmovingthingswhenanimationfinished = 1;
	return 1;
      }
      if(c == 'n'){
	getchar();
	return 1;
      }
    }
  }

  if(strcmp(s, "aniinfo") == 0) {
    printf("現在のアニメーション枚数: %d\n",numofanimation);
    return 1;
  }

  if (strcmp(s, "save") == 0) {
    s = strtok(NULL, " ");
    save_history(s, *begin_p);
    return 1;
  }

  if(strcmp(s, "load") == 0) {
    s = strtok(NULL, " ");
    *begin_p = load_history(s, *begin_p, hsize);
    Node *p = *begin_p;
    while(p != NULL){
      interpret_command(p->str, NULL, begin_p, fp);
      p = p->next;
    }
    return 1;
  }
  
  if (strcmp(s, "undo") == 0) {
    //アニメ用に再利用したいのでnull想定
    if(hsize != NULL){
      if(*hsize <= 0){
	printf("error: too many undo.\n");
	return 1;
      }
    }
    
    init_canvas();
    *begin_p = pop_back(*begin_p);
    Node *p = *begin_p;
    while(p != NULL){
      interpret_command(p->str, NULL, begin_p, fp);
      p = p->next;
    }
    if(hsize!=NULL){
      (*hsize)--;
    }
    return 1;
  }

  if (strcmp(s, "quit") == 0) {
    return 2;
  }

  printf("error: unknown command.\n");

  return 1;
}


void load_information()
{
  FILE *fp;
  char s1[50],s2[50];
  if((fp = fopen("animation/information.txt","r")) == NULL){
    numofanimation = 0;
    numofmovingthingswhenanimationfinished = 1;
    return;
  }
  fgets(s1, 50, fp);
  numofanimation = atoi(s1);
  fgets(s2, 50, fp);
  numofmovingthingswhenanimationfinished = atoi(s2);
  fclose(fp);
}

void save_number_of_animations()
{
  FILE *fp;
  fp = fopen("animation/information.txt","w");
  fprintf(fp,"%d\n",numofanimation);
  fclose(fp);
}

void save_number_of_moving_things()
{
  FILE *fp;
  fp = fopen("animation/information.txt","a");
  fprintf(fp,"%d\n",numofmovingthingswhenanimationfinished);
  fclose(fp);
}

int deleteanimationfile()
{
  char filename[32];
  int i;
  for(i=0; i<numofanimation; i++){
    sprintf(filename,"animation/animation%d.txt", i);
    if(remove(filename) != 0){
      printf("ファイル削除に失敗しました\n");
      return 1;
    }
  }
  if(remove("animation/information.txt") != 0){
    printf("ファイル削除に失敗しました\n");
    return 1;
  }
  return 0;
}

int main()
{
  const char *filename = "canvas.txt";
  FILE *fp;
  char buf[BUFSIZE];

  if ((fp = fopen(filename, "a")) == NULL) {
    fprintf(stderr, "error: cannot open %s.\n", filename);
    return 1;
  }

  init_canvas();
  print_canvas(fp);
  
  int hsize = 0;
  Node *begin = NULL;
  load_information();
  
  while(1){
    printf("%d > ", hsize);
    fgets(buf, BUFSIZE, stdin);

    const int r = interpret_command(buf, &hsize, &begin, fp);
    if (r == 2) break;
    if (r == 3) {
      numofmovingthingswhenanimationfinished = 1;
      save_number_of_animations();
      save_number_of_moving_things();
    }
    if (r == 4) {
      save_number_of_animations();
      save_number_of_moving_things();
    }
    if (r == 0) {
      begin = push_back(begin, buf);
      hsize++;
    }
    print_canvas(fp);
  }

  fclose(fp);

  return 0;
}
