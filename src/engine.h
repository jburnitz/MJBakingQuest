#ifndef ENGINE_H
#define ENGINE_H

#include <QtCore>
#include <QtGui>

#if QT_VERSION >= 0x050000
    #include <QtWidgets>
#endif

#include "objects.h"
#include "parser.h"
#include "objStructure.h"
#include "definitions.h"

class engine
{

public:
    engine();
    engine( QGraphicsScene *scene );
    ~engine();

    QGraphicsScene* GetScene();
    void SetScene( QGraphicsScene *scene );
    void SetParentWindow(QWidget *pWindow );
    void loadGame(QString level);
    void saveGame(QString name);
    void ClickedOpenMap(void);
    void ClickedSaveMap(void);
    void CloseMap(void);
    void ClickedDrawGridLines(void);

    void AddSprite(const char *spriteFName, int xLoc, int yLoc );
    void moveChar(int direction);
    void moveEnemies();
    void moveGood();
    void getBlock();
    void dropBlock();
    void loadNext();
    void checkCollisions();
    void startOver();

    //made mj and the array of blocks public, might change it back to private later if that is better
    Node *mj;
    Node *walkable[20][30];
    QGraphicsRectWidget *goodObj[5];
    bool mjHasBlock;
    int itemCount;
    int life;
    objStructure *blocks;
    parser *parsley;

private:
    const QRect *sSize;
    objStructure *goodGuys;
    objStructure *enemies;

    objStructure *other;
    objStructure *doors;

    QBrush *brush;
    QString newName;
    QMediaPlayer *player;
    QGraphicsScene *uiScene;
    QWidget *parentWindow;
    QGraphicsRectWidget *hearts[3];

    //variables used for moving and facing MJ in the right place
    int facing;
    int prevFacing;

    int curItems;
    bool safeToCheckEnemyCollision;

    void DrawGrid(QGraphicsScene *scene);
    void MoveBlock(QGraphicsWidget *box, QGraphicsScene *scene, int x, int y);
    void setNewName (QString subName);
    void setBrush();
    int LoadMap(QGraphicsScene *scene);
    int LoadMap(QGraphicsScene *scene, QString fileName);
    void reset(QString level);


};

#endif // ENGINE_H
