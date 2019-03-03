#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "defines.h"
#include "cuiEdit.h"

// 初期化
int init(int argc, char *argv[], BUF *buf, SCREEN *scr){
  FILE *fp;
  char c;
  BUF *next;
  // 行関連初期化
  buf->next = (BUF *)malloc(sizeof(BUF));
  buf->prev = buf->next;
  buf->next->prev = buf;
  buf->next->next = buf;
  buf->cols = -1;
  buf->maxCols = -1;
  buf->lineNum = 0;
  // 文字関連初期化
  buf->next->c = (char *)malloc(sizeof(char) * STRBUF);
  buf->next->cols = 0;
  buf->next->maxCols = STRBUF;
  buf->next->lineNum = 1;
  for(int i = 0; i < STRBUF; i++){
    buf->next->c[i] = '\0';
  }
  scr->top = buf->next;
  scr->leftCols = 0;
  scr->otherLine = DEBUG == 0 ? 1 : 2;
  scr->fPosX = scr->fPosY = 0;
  scr->curX = 7;
  scr->curY = scr->otherLine;
  for(int i = 0; i < 256; i++) scr->filename[i] = '\0';
  if(argc > 1){
    for(int i = 0; i < strlen(argv[1]); i++){
      scr->filename[i] = argv[1][i];
    }
    
    if((fp = fopen(scr->filename, "r")) != NULL){
      next = buf->next;
      int count = 0;
      while((c = fgetc(fp)) != EOF){
        if(c == '\n'){
          next = insertNewLine(next);
          count = 0;
        }else {
          if(next->cols + 1 > next->maxCols){
            addLineStr(next);
          }
          next->c[count++] = c;
          next->cols++;
        }
      }
      fclose(fp);
    }
  }
  return 0;
}

// 終了
void end(BUF *buf){
  BUF *next, *del;
  next = buf->next;
  while(next != buf){
    free(next->c);
    del = next;
    next = next->next;
    free(del);
  }
  
}

void dispBuffer(SCREEN *scr){
  BUF *next;
  next = scr->top;
  int n = 0;
  char mes[] = "@editor->InsertMode->";
  attrset(COLOR_PAIR(1));
  printw(mes);
  attrset(COLOR_PAIR(2));
  for(int i = 0; i < scr->width - strlen(mes)-1; i++){
    printw("-");
  }
  printw("\n");
  attrset(COLOR_PAIR(0));
  
  if(DEBUG) printw("w:%d h:%d lC:%d cX%d xY:%d cols:%d\n", scr->width, scr->height, scr->leftCols, scr->curX, scr->curY, scr->top->cols);
  while(next->lineNum != 0){
    if(n == scr->height - scr->otherLine){
      break;
    }
    printw("[%04d] ", next->lineNum);
    for(int i = scr->leftCols; i < scr->leftCols + scr->width - 8; i++){
      if(next->c[i] == '\0') break;
      printw("%c", next->c[i]);
    }
    printw("\n");
    next = next->next;
    n++;
  }
  refresh();
}

// 最大文字数を増やす
void addLineStr(BUF *b){
  char *tmp, *strdel;
  int i;
  tmp = (char *)malloc(sizeof(char) * (STRBUF + b->maxCols));
  if(tmp == NULL){
  }
  b->maxCols += STRBUF;
  
  for(i = 0; i < b->cols; i++){
    tmp[i] = b->c[i];
  }
  for(i = b->cols; i < b->maxCols; i++){
    tmp[i] = '\0';
  }
  strdel = b->c;
  b->c = tmp;
  free(strdel);
  
}

// 現在の行の次に新しい行を挿入
BUF *insertNewLine(BUF *now){
  BUF *next;
  
  next = (BUF *)malloc(sizeof(BUF));
  next->c = (char *)malloc(sizeof(char) * STRBUF);
  for(int i = 0; i < STRBUF; i++){
    next->c[i] = '\0';
  }
  next->cols = 0;
  next->maxCols = STRBUF;
  next->lineNum = now->lineNum + 1;
  
  next->next = now->next;
  next->prev = now;
  now->next = next;
  now->next->next->prev = next;
  
  now = next;
  
  while(next->lineNum != 0){
    next->lineNum =next->prev->lineNum + 1;
    next = next->next;
  }
  return now;
}

BUF *keyOperation(BUF *buf, BUF *now, SCREEN *scr, int state){

  switch(state){
  case CUR_LEFT:
    if(scr->fPosX > 0){
      if(scr->leftCols > 0 && scr->curX == 7){
        scr->leftCols--;
      }else if(scr->curX > 7) scr->curX--;
      scr->fPosX--;
    }else if(now->prev != buf){
      scr->fPosX = now->prev->cols;
      scr->fPosY--;
      if(scr->curY == 1) scr->top = scr->top->prev;
      else if(scr->curY > 1) scr->curY--;
      if(now->prev->cols > 0){  // 前の行に文字があるなら
          if(now->prev->cols + 7 > scr->width) scr->curX = 7 + now->prev->cols; // カーソル位置を設定
          else scr->curX = scr->width-1;
          scr->fPosX = now->prev->cols;
        }else {
          scr->curX = 7;
          scr->fPosX = 0;
        }
      scr->curX = now->prev->cols + 7;
      now = now->prev;
      
      scr->leftCols = 0;
      if(now->cols > scr->width){
        scr->leftCols = now->cols - scr->width + 8;
        scr->curX = scr->width-1;
      }
    }
    break;
  case CUR_RIGHT:
    if(scr->fPosX < now->cols){
      if(scr->curX < scr->width-1){
        scr->curX++;
      }else {
        scr->leftCols++;
      }
      scr->fPosX++;
    }else if(now->next != buf){ // あれば次の行へ
      scr->fPosX = 0;
      scr->fPosY++;
      if(scr->curY+1 < scr->height) scr->curY++;
      else scr->top = scr->top->next;
      scr->curX = 7;
      now = now->next;  
      scr->leftCols = 0;
    }
    break;
  case CUR_UP:
    if(scr->fPosY > 0){
      if(scr->curY == 1) scr->top = scr->top->prev;
      else if(scr->curY > 1) scr->curY--;
      scr->fPosY--;
      if(scr->fPosX > now->prev->cols && now->prev != buf){
        scr->leftCols = now->prev->cols - scr->width > 0 ? now->prev->cols - scr->width : 0; 
        scr->fPosX = now->prev->cols;
        scr->curX = now->prev->cols + 7 > scr->width ? scr->width-1 : now->prev->cols + 7;
      }

      now = now->prev;
    }
    break;
  case CUR_DOWN:
    if(now->next != buf){
      if(scr->curY+1 < scr->height) scr->curY++;
      else scr->top = scr->top->next;
      scr->fPosY++;
      if(scr->fPosX > now->next->cols && now->next != buf){
        scr->leftCols = 0;
        scr->fPosX = now->next->cols;
        scr->curX = now->next->cols + 7;
      }
      now = now->next;
    }
    break;
  case CUR_TAB:
    if(now->cols + 2 < now->maxCols){
      addLineStr(now);
    }
    if(scr->fPosX < now->cols){
      for(int i = now->cols - 1; i >= scr->fPosX; i--){
        now->c[i+2] = now->c[i];
      }
    }
    
    for(int i = 0; i < 2; i++){
      now->c[scr->fPosX++] = ' '; 
      now->cols++;
      if(scr->curX < scr->width-1){
        scr->curX++;
      }else {
        scr->leftCols++;
      }
    }
  }
  return now;
}
