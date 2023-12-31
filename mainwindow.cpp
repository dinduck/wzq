#include "mainwindow.h"
#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QSound>
#include <QTimer>
#include <math.h>

// -------全局遍历-------//
#define CHESS_ONE_SOUND "./res/sound/chessone.wav"
#define WIN_SOUND "./res/sound/win.wav"
#define LOSE_SOUND "./res/sound/lose.wav"

const int kBoardMargin = 30; // 棋盘边缘空隙
const int kRadius = 15;      // 棋子半径
const int kMarkSize = 6;     // 落子标记边长
const int kBlockSize = 40;   // 格子的大小
const int kPosDelta = 20;    // 鼠标点击的模糊距离上限

const int kAIDelay = 700; // AI下棋的思考时间

// -------------------- //

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  // 设置棋盘大小
  setFixedSize(kBoardMargin * 2 + kBlockSize * kBoardSizeNum,
               kBoardMargin * 2 + kBlockSize * kBoardSizeNum);
  //    setStyleSheet("background-color:yellow;");

  // 开启鼠标hover功能
  setMouseTracking(true);
  //    centralWidget()->setMouseTracking(true);

  // 添加菜单
  QMenu *gameMenu = menuBar()->addMenu(tr("Game"));
  QAction *actionPVP = new QAction("Person VS Person", this);
  connect(actionPVP, SIGNAL(triggered()), this, SLOT(initPVPGame()));
  gameMenu->addAction(actionPVP);

  QAction *actionPVE = new QAction("Person VS Computer", this);
  connect(actionPVE, SIGNAL(triggered()), this, SLOT(initPVEGame()));
  gameMenu->addAction(actionPVE);

  // 开始游戏
  initGame();
}

MainWindow::~MainWindow() {
  if (game) {
    delete game;
    game = nullptr;
  }
}

void MainWindow::initGame() {
  // 初始化游戏模型
  game = new GameModel;
  initPVPGame();
}

void MainWindow::initPVPGame() {
  game_type = PERSON;
  game->gameStatus = PLAYING;
  game->startGame(game_type);
  update();
}

void MainWindow::initPVEGame() {
  game_type = BOT;
  game->gameStatus = PLAYING;
  game->startGame(game_type);
  update();
}

void MainWindow::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  // 绘制棋盘
  painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿
  //    QPen pen; // 调整线条宽度
  //    pen.setWidth(2);
  //    painter.setPen(pen);
  for (int i = 0; i < kBoardSizeNum + 1; i++) {
    painter.drawLine(kBoardMargin + kBlockSize * i, kBoardMargin,
                     kBoardMargin + kBlockSize * i,
                     size().height() - kBoardMargin);
    painter.drawLine(kBoardMargin, kBoardMargin + kBlockSize * i,
                     size().width() - kBoardMargin,
                     kBoardMargin + kBlockSize * i);
  }

  QBrush brush;
  brush.setStyle(Qt::SolidPattern);
  // 绘制落子标记(防止鼠标出框越界)
  if (clickPosRow > 0 && clickPosRow < kBoardSizeNum && clickPosCol > 0 &&
      clickPosCol < kBoardSizeNum &&
      game->gameMapVec[clickPosRow][clickPosCol] == 0) {
    if (game->playerFlag)
      brush.setColor(Qt::white);
    else
      brush.setColor(Qt::black);
    painter.setBrush(brush);
    painter.drawRect(kBoardMargin + kBlockSize * clickPosCol - kMarkSize / 2,
                     kBoardMargin + kBlockSize * clickPosRow - kMarkSize / 2,
                     kMarkSize, kMarkSize);
  }

  // 绘制棋子
  for (int i = 0; i < kBoardSizeNum; i++)
    for (int j = 0; j < kBoardSizeNum; j++) {
      if (game->gameMapVec[i][j] == 1) {
        brush.setColor(Qt::white);
        painter.setBrush(brush);
        painter.drawEllipse(kBoardMargin + kBlockSize * j - kRadius,
                            kBoardMargin + kBlockSize * i - kRadius,
                            kRadius * 2, kRadius * 2);
      } else if (game->gameMapVec[i][j] == -1) {
        brush.setColor(Qt::black);
        painter.setBrush(brush);
        painter.drawEllipse(kBoardMargin + kBlockSize * j - kRadius,
                            kBoardMargin + kBlockSize * i - kRadius,
                            kRadius * 2, kRadius * 2);
      }
    }

  // 判断输赢
  if (clickPosRow > 0 && clickPosRow < kBoardSizeNum && clickPosCol > 0 &&
      clickPosCol < kBoardSizeNum &&
      (game->gameMapVec[clickPosRow][clickPosCol] == 1 ||
       game->gameMapVec[clickPosRow][clickPosCol] == -1)) {
    if (game->isWin(clickPosRow, clickPosCol) && game->gameStatus == PLAYING) {
      qDebug() << "win";
      game->gameStatus = WIN;
      QSound::play(WIN_SOUND);
      QString str;
      if (game->gameMapVec[clickPosRow][clickPosCol] == 1)
        str = "white player";
      else if (game->gameMapVec[clickPosRow][clickPosCol] == -1)
        str = "black player";
      QMessageBox::StandardButton btnValue =
          QMessageBox::information(this, "congratulations", str + " win!");

      // 重置游戏状态，否则容易死循环
      if (btnValue == QMessageBox::Ok) {
        game->startGame(game_type);
        game->gameStatus = PLAYING;
      }
    }
  }

  // 判断死局
  if (game->isDeadGame()) {
    QSound::play(LOSE_SOUND);
    QMessageBox::StandardButton btnValue =
        QMessageBox::information(this, "oops", "dead game!");
    if (btnValue == QMessageBox::Ok) {
      game->startGame(game_type);
      game->gameStatus = PLAYING;
    }
  }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
  // 通过鼠标的hover确定落子的标记
  int x = event->x();
  int y = event->y();

  // 棋盘边缘不能落子
  if (x >= kBoardMargin + kBlockSize / 2 && x < size().width() - kBoardMargin &&
      y >= kBoardMargin + kBlockSize / 2 &&
      y < size().height() - kBoardMargin) {
    // 获取最近的左上角的点
    int col = x / kBlockSize;
    int row = y / kBlockSize;

    int leftTopPosX = kBoardMargin + kBlockSize * col;
    int leftTopPosY = kBoardMargin + kBlockSize * row;

    // 根据距离算出合适的点击位置,一共四个点，根据半径距离选最近的
    clickPosRow = -1; // 初始化最终的值
    clickPosCol = -1;
    int len = 0; // 计算完后取整就可以了

    // 确定一个误差在范围内的点，且只可能确定一个出来
    len = sqrt((x - leftTopPosX) * (x - leftTopPosX) +
               (y - leftTopPosY) * (y - leftTopPosY));
    if (len < kPosDelta) {
      clickPosRow = row;
      clickPosCol = col;
    }
    len = sqrt((x - leftTopPosX - kBlockSize) * (x - leftTopPosX - kBlockSize) +
               (y - leftTopPosY) * (y - leftTopPosY));
    if (len < kPosDelta) {
      clickPosRow = row;
      clickPosCol = col + 1;
    }
    len = sqrt((x - leftTopPosX) * (x - leftTopPosX) +
               (y - leftTopPosY - kBlockSize) * (y - leftTopPosY - kBlockSize));
    if (len < kPosDelta) {
      clickPosRow = row + 1;
      clickPosCol = col;
    }
    len = sqrt((x - leftTopPosX - kBlockSize) * (x - leftTopPosX - kBlockSize) +
               (y - leftTopPosY - kBlockSize) * (y - leftTopPosY - kBlockSize));
    if (len < kPosDelta) {
      clickPosRow = row + 1;
      clickPosCol = col + 1;
    }
  }

  update();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
  // 人下棋
  if (!(game_type == BOT && !game->playerFlag)) {
    chessOneByPerson();
    // BOT 下棋
    if (game->gameType == BOT && !game->playerFlag) {
      // 延时
      QTimer::singleShot(kAIDelay, this, SLOT(chessOneByAI()));
    }
  }
}

void MainWindow::chessOneByPerson() {
  // 根据当前存储的坐标下子
  // 只有有效点击才下子，并且该处没有子
  if (clickPosRow != -1 && clickPosCol != -1 &&
      game->gameMapVec[clickPosRow][clickPosCol] == 0) {
    game->actionByPerson(clickPosRow, clickPosCol);
    QSound::play(CHESS_ONE_SOUND);

    update();
  }
}

void MainWindow::chessOneByAI() {
  game->actionByAI(clickPosRow, clickPosCol);
  QSound::play(CHESS_ONE_SOUND);
  update();
}
