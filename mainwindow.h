#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "GameModel.h"
#include <QMainWindow>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  // 绘制
  void paintEvent(QPaintEvent *event);
  // 鼠标移动事件
  void mouseMoveEvent(QMouseEvent *event);
  // 鼠标释放事件
  void mouseReleaseEvent(QMouseEvent *event);

private:
  GameModel *game;              // 游戏指针
  GameType game_type;           // 存储游戏类型
  int clickPosRow, clickPosCol; // 存储将点击的位置
  void initGame();
  void checkGame(int y, int x);

private slots:
  void chessOneByPerson(); // 人执行
  void chessOneByAI();     // AI下棋

  void initPVPGame();
  void initPVEGame();
};

#endif // MAINWINDOW_H
