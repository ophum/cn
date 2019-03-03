#ifndef DEF_STRUCTS_H
#define DEF_STRUCTS_H

typedef struct chars CH;
struct chars{
  CH *next;
  CH *prev;
  char c;
};

typedef struct buffer BUF;
struct buffer{
  BUF *next;
  BUF *prev;
  char *c;
  //CH str;
  int maxCols;
  int cols;
  int lineNum;
};

typedef struct screen SCREEN;
struct screen{
  BUF *top; // 一番上に表示する行のBUF
  int leftCols; // 何文字目から表示するか
  int otherLine;  // ステータス表示行数
  int width;      // 端末横幅
  int height;     // 端末縦幅
  int curX, curY; // カーソルの位置
  int fPosX, fPosY; // データの位置
  
  char filename[256];   // 開いている/保存するファイル名
};

#endif /* DEF_STRUCTS_H */
