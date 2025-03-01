/*! \abstract engine
 *         This class is the engine for the whole project. The engine is where the parser (which parses the levels that are loaded) is called
 *         and the objects (characters, scenery, etc.) are created. It also defines the different options for the game, start new game,
 *         save game, or continue a game.
 */

/*
 * engine.cpp
 * Originally Authored by Joseph Burnitz
 * renders the game
 *
 * TODO: as of 10/24/2013
 * *Remove the need of mainwindow.h
 * *Seperate out the file reading part
 * *Make a real, robust level parser; possibly put into own class
 *   *Make datastructure class for keeeping track of the objects generated
*/

#include "engine.h"
#include <iostream>
#define BLOCK_SIZE ( 30 )

/*! \brief engine::engine
 *         Creates all objects defined in the parser for each level. It also keeps track of how many walkable objects (MJ, good guys, enemies) and hearts
 *        (for the health bar) are to appear in the level.
 */
engine::engine(){
    goodGuys = new objStructure();
    enemies = new objStructure();
    blocks = new objStructure();
    other = new objStructure();
    doors = new objStructure();
    parsley = new parser();
    facing = 0;
    prevFacing = 0;
    itemCount = 0;
    curItems = 0;
    mjHasBlock = false;
    player = new QMediaPlayer;
    safeToCheckEnemyCollision = true;
    //initialize array that holds politers to walkable blocks
    //useful for moving
    for(int y = 0; y< 20; y++){
        for(int x = 0; x<30; x++)
            walkable[y][x] = NULL;
    }

    for(int x = 0; x<5; x++)
        goodObj[x] = NULL;
    for(int x = 0; x<3; x++)
        hearts[x] = NULL;

}

engine::engine( QGraphicsScene *scene ){
    uiScene = scene;
}

engine::~engine(){
    delete sSize;
    delete uiScene;
    delete goodGuys;
    delete enemies;
    delete blocks;
    delete other;
    delete walkable;
    delete parsley;
}

/*! \brief engine::GetScene
 *         returns a pointer to the current graphics scene
 */
QGraphicsScene* engine::GetScene(){
    return uiScene;
}

void engine::SetScene(QGraphicsScene *scene){
    uiScene = scene;
}

/*! \brief engine::AddSprite
 *         Creates a widget for sprites, givien a sprite picture, and a position. The sprite also holds properties that tell wheter or not it is a movable
 *         object.
 */
void engine::AddSprite(const char* spriteFName, int xLoc, int yLoc ){
    QGraphicsRectWidget *tmp = new QGraphicsRectWidget( spriteFName, BLOCK_SIZE, BLOCK_SIZE );
    tmp->setFlag(QGraphicsItem::ItemIsMovable, true);
    //tmp->setFlag(QGraphicsItem::ItemIsSelectable, true);
    MoveBlock(tmp, uiScene, xLoc, yLoc );
    tmp->setCursor(Qt::OpenHandCursor);
    uiScene->addItem(tmp);
}

void engine::SetParentWindow( QWidget *pWindow ){
    parentWindow = pWindow;
}

/*! \brief engine::MoveBlock
 * Moves a block by offset of their size
 * 0,0 is in the lower left, e.g. Quadrant 1
*/
void engine::MoveBlock(QGraphicsWidget *box, QGraphicsScene *scene, int x, int yOffset){
    box->moveBy(BLOCK_SIZE*x,scene->height()-BLOCK_SIZE*yOffset);
}

/*! \brief engine::DrawGrid
 * Draws a grid lines through the scene for line up / snap to purposes
 */
void engine::DrawGrid(QGraphicsScene *scene){
    int gridWidth = (scene->width())/BLOCK_SIZE;
    int gridHeight = (scene->height())/BLOCK_SIZE;

    for(int x=1; x<=gridWidth; x++){
        scene->addLine(BLOCK_SIZE*x, 0, BLOCK_SIZE*x, BLOCK_SIZE*gridHeight,QPen(Qt::red));
    }
    for(int y=gridHeight; y>=0; y--){
        scene->addLine(0, BLOCK_SIZE*y, BLOCK_SIZE*gridWidth, BLOCK_SIZE*y,QPen(Qt::red));
    }
}

/*! \brief engine::LoadMap
 * Loads a map into the Graphics Scene
 * Open a file chooser dialog
 */
int engine::LoadMap(QGraphicsScene *scene){
    parsley->readFile( parentWindow, goodGuys, enemies, blocks, doors,other, NULL );

    Node *tmp = goodGuys->head;
    while(tmp != 0){
        QString spriteName("sprites/");
        spriteName.append(tmp->location.trimmed());
        spriteName.append(".png");

        tmp->sprite = new QGraphicsRectWidget(QPixmap(spriteName), BLOCK_SIZE, BLOCK_SIZE);
        MoveBlock(tmp->sprite, scene, tmp->x, tmp->y);
        //std::cout<< tmp->x<< std::endl << tmp->y << std::endl;
        scene->addItem(tmp->sprite);
        //asign node to MJ
        if(tmp->blockType.compare(QString("MJ")) == 0){
            mj = tmp;
        }
        tmp = tmp->next;
    }

    tmp = enemies->head;
    while(tmp != 0){
        QString spriteName("sprites/");
        spriteName.append(tmp->location.trimmed());
        spriteName.append(".png");

        tmp->sprite = new QGraphicsRectWidget(QPixmap(spriteName), BLOCK_SIZE, BLOCK_SIZE);
        MoveBlock(tmp->sprite, scene, tmp->x, tmp->y);
        scene->addItem(tmp->sprite);
        tmp = tmp->next;
    }

    tmp = blocks->head;
    while(tmp != 0){
        QString spriteName("sprites/");
        spriteName.append(tmp->location.trimmed());
        spriteName.append(".png");

        tmp->sprite = new QGraphicsRectWidget(QPixmap(spriteName), BLOCK_SIZE, BLOCK_SIZE);
        MoveBlock(tmp->sprite, scene, tmp->x, tmp->y);
        scene->addItem(tmp->sprite);
        //walkable[tmp->x][tmp->y-1] = tmp; Might no be needed here... Not sure
        tmp = tmp->next;
    }

    tmp = other->head;
    while(tmp != 0){

        QString spriteName("sprites/");
        spriteName.append(tmp->location.trimmed());
        spriteName.append(".png");

        if(tmp->blockType.compare( QString("BACKGROUND")) == 0)
            scene->setBackgroundBrush(QBrush(Qt::black, QPixmap(spriteName)));
        else{
            tmp->sprite = new QGraphicsRectWidget(QPixmap(spriteName), BLOCK_SIZE, BLOCK_SIZE);
            MoveBlock(tmp->sprite, scene, tmp->x, tmp->y);
            scene->addItem(tmp->sprite);
            tmp->sprite->setZValue(-1);
        }
        tmp = tmp->next;
    }

    tmp = doors->head;
    while(tmp != 0){

        QString spriteName("sprites/");
        spriteName.append(tmp->location.trimmed());
        spriteName.append(".png");

        if(tmp->blockType.compare( QString("BACKGROUND")) == 0)
            scene->setBackgroundBrush(QBrush(Qt::black, QPixmap(spriteName)));
        else{
            tmp->sprite = new QGraphicsRectWidget(QPixmap(spriteName), BLOCK_SIZE, BLOCK_SIZE);
            MoveBlock(tmp->sprite, scene, tmp->x, tmp->y);
            scene->addItem(tmp->sprite);
            tmp->sprite->setZValue(-1);
        }
        tmp = tmp->next;
    }

    return 1;
}

/*! \brief engine::LoadMap
 * Loads the level file specified by the fileName
 * Useful for autoloading the next level upon winning
 * it's almost identical to the one above it
 */
int engine::LoadMap(QGraphicsScene *scene, QString fileName){
    parsley->readFile(parentWindow, goodGuys, enemies, blocks, doors,other, fileName );

    life = parsley->lives;
    for(int x =0; x<life; x++){
        hearts[x] = new QGraphicsRectWidget(QPixmap("sprites/heart.png"), BLOCK_SIZE, BLOCK_SIZE);
        MoveBlock(hearts[x], uiScene, 27+x, 20);
        uiScene->addItem(hearts[x]);
    }

    Node *tmp = goodGuys->head;
    while(tmp != 0){
        QString spriteName("sprites/");
        spriteName.append(tmp->location.trimmed());
        spriteName.append(".png");

        tmp->sprite = new QGraphicsRectWidget(QPixmap(spriteName), BLOCK_SIZE, BLOCK_SIZE);
        MoveBlock(tmp->sprite, scene, tmp->x, tmp->y);
        scene->addItem(tmp->sprite);

        //asign node to MJ
        if(tmp->blockType.compare(QString("MJ")) == 0)
            mj = tmp;
        else{
            //set movement to either left or right
            QTime time = QTime::currentTime();
            qsrand((uint)time.msec());
            tmp->movement = qrand() %(2);

            //update item count
            if(tmp->hasObj)
                itemCount ++;
            //draw object that mj already has from saved game
            else if (tmp->hasObj == false){
                goodObj[curItems] = new QGraphicsRectWidget(QPixmap(tmp->goodObj), BLOCK_SIZE, BLOCK_SIZE);
                MoveBlock(goodObj[curItems], uiScene, curItems, 20);
                uiScene->addItem(goodObj[curItems]);
                curItems ++;
            }
        }
        tmp = tmp->next;
    }

    tmp = enemies->head;
    while(tmp != 0){
        QString spriteName("sprites/");
        spriteName.append(tmp->location.trimmed());
        spriteName.append(".png");

        tmp->sprite = new QGraphicsRectWidget(QPixmap(spriteName), BLOCK_SIZE, BLOCK_SIZE);
        MoveBlock(tmp->sprite, scene, tmp->x, tmp->y);
        scene->addItem(tmp->sprite);

        //set movement to either left or right
        QTime time = QTime::currentTime();
        qsrand((uint)time.msec());
        tmp->movement = qrand() %(2);

        tmp = tmp->next;
    }

    tmp = blocks->head;
    while(tmp != 0){
        QString spriteName("sprites/");
        spriteName.append(tmp->location.trimmed());
        spriteName.append(".png");

        tmp->sprite = new QGraphicsRectWidget(QPixmap(spriteName), BLOCK_SIZE, BLOCK_SIZE);
        MoveBlock(tmp->sprite, scene, tmp->x, tmp->y);
        scene->addItem(tmp->sprite);
        walkable[tmp->y-1][tmp->x] = tmp;
        tmp = tmp->next;
    }

    tmp = other->head;
    while(tmp != 0){

        QString spriteName("sprites/");
        spriteName.append(tmp->location.trimmed());
        spriteName.append(".png");

        if(tmp->blockType.compare( QString("BACKGROUND")) == 0)
            scene->setBackgroundBrush(QBrush(Qt::black, QPixmap(spriteName)));
        else{
            tmp->sprite = new QGraphicsRectWidget(QPixmap(spriteName), BLOCK_SIZE, BLOCK_SIZE);
            MoveBlock(tmp->sprite, scene, tmp->x, tmp->y);
            scene->addItem(tmp->sprite);
            tmp->sprite->setZValue(-1);
        }
        tmp = tmp->next;
    }

    tmp = doors->head;
    while(tmp != 0){

        QString spriteName("sprites/");
        spriteName.append(tmp->location.trimmed());
        spriteName.append(".png");

        if(tmp->blockType.compare( QString("BACKGROUND")) == 0)
            scene->setBackgroundBrush(QBrush(Qt::black, QPixmap(spriteName)));
        else{
            tmp->sprite = new QGraphicsRectWidget(QPixmap(spriteName), BLOCK_SIZE, BLOCK_SIZE);
            MoveBlock(tmp->sprite, scene, tmp->x, tmp->y);
            scene->addItem(tmp->sprite);
            tmp->sprite->setZValue(-1);
        }
        tmp = tmp->next;
    }

    return 1;
}

/*! \brief engine::saveGame
 * Created a file with the current map and status of the game.
 * This method does the reverse of LoadMap where instead od loading the objects from the map file, it creates a map file with the object in the level and the
 * the states they are in.
 */
void engine::saveGame(QString name){
    parsley->lives = life;
    parsley->createFile(name, goodGuys, enemies, blocks, doors,other);
}

/*! \brief engine::loadGame
 *loads level without the file chooser, for now default level
 */
void engine::loadGame(QString level){
    LoadMap(uiScene, level);
}

/*! \brief engine::ClickedOpenMap
 *Opens the file chooser dialog and loads the map
 */
void engine::ClickedOpenMap(void){
    LoadMap(uiScene);
}

/*! \brief engine::ClickedSaveMap
 * Opens the file save dialog and allows user to save current level to a new file, or overwrite an already saved file
 */
void engine::ClickedSaveMap(void){

    QString text = QInputDialog::getText( parentWindow, "Save Level", "Level name:", QLineEdit::Normal, "levelname", NULL );

    QFile file( QString("levels/").append(text) );

    if(file.exists()){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question( parentWindow, "Level Exists!", "Overwrite?", QMessageBox::Yes|QMessageBox::No);

        if( reply == QMessageBox::Yes)
        {
          qDebug() << "yes";
          this->saveGame( text );
        }
    }
    else{
        //file doesn't exist sooo
        this->saveGame( text );
    }
}

/*! \brief engine::CloseMap
 * Clears out the scene
 */
void engine::CloseMap(void){
    //AskToSave...
    qDeleteAll( uiScene->items() );
    uiScene->setBackgroundBrush(QBrush(Qt::white));
}

void engine::ClickedDrawGridLines(void){
    DrawGrid( uiScene );
}

void engine::setBrush(){
    brush = new QBrush( QPixmap(newName) );
}

/*! \brief engine::setNewName
 * Sets sprite name so that picture file can be accessed and loaded
 */
void engine::setNewName (QString subName){
    newName = "sprites/";
    newName.append(subName);
    newName.append(".png");
}

/*! \brief engine::loadNext
 *  checks if mj is by the door if she is then load the next level
 */
void engine::loadNext(){
    //check if mj is by the door if she is then load the next level
    Node *tmp = doors->head;
    while(tmp != NULL){
        if( (tmp->x == mj->x) && (tmp->y == mj->y)){
            life = 0;

            QMessageBox msgBox;
            msgBox.setText("Next Level loading...");
            msgBox.exec();

            reset(parsley->nextLevel);
            break;
        }
        tmp = tmp->next;
    }
}

/*! \brief engine::moveChar
 *     starts the current level over by first clearing out the life and then callong reset
 */
void engine::startOver(){
    life =0;
    reset(parsley->curLevel);
}

/*! \brief engine::reset
 * resets the level by seting objects totheir orignal positions and states
 * cannot be called when animation is still taking place
 */
void engine::reset(QString level){
    //Error check on level
    if(level == NULL || level.length() == 0){
        std::cout << "the string that was passed to reset() is not acceptable\n";
        return;
   }
    //empty the linked list and remove the graphic objects
    blocks->removeAll();
    other->removeAll();
    enemies->removeAll();
    goodGuys->removeAll();
    doors->removeAll();

    //reset userstats
    mj = NULL;
    facing = 0;
    prevFacing = 0;
    itemCount = 0;
    mjHasBlock = false;

    //initialize array that holds politers to walkable blocks
    //useful for moving
    for(int y = 0; y< 20; y++){
        for(int x = 0; x<30; x++)
            walkable[y][x] = NULL;
    }

    //reset and remove goodobj
    for(int x = 0; x<5; x++){
        if(goodObj[x] != NULL)
            delete goodObj[x];
        goodObj[x] = NULL;
    }
    //reset and remove hearts
    for(int x = 0; x<3; x++){
        if(hearts[x] != NULL)
            delete hearts[x];
        hearts[x] = NULL;
    }

    loadGame(level);
}

/*! \brief engine::moveChar
 *     method that is used to move mj.
 *     This method changes her sprite picture and updates her posistion
 */
void engine::moveChar(int direction){

    int mjPrevX = mj->x;
    int mjPrevY = mj->y;
    if(facing == -direction || facing == 0)
        facing = facing + direction;

    //face left
    if (facing == -1){
        if (mjHasBlock)
            setNewName("MJ_move_left_up");
        else
            setNewName("MJ_move_left");
    }
    //face right
    else if (facing == 1){
        if (mjHasBlock){
            setNewName("MJ_move_right_up");
        }
        else
            setNewName("MJ_move");
    }
    //face foward
    else if (facing == 0){
        if (mjHasBlock)
            setNewName("MJ_left_up");
        else
        setNewName("MJ_left");
    }
    mj->sprite->brush=new QBrush( QPixmap(newName) );
    mj->sprite->update();

    //move left
    if(direction < 0 && prevFacing == facing){

        //going down
        if(walkable[mj->y-1][mj->x-1] == NULL && walkable[mj->y-2][mj->x-1] == NULL && walkable[mj->y-3][mj->x-1] != NULL){
            mj->sprite->moveBy(-BLOCK_SIZE, BLOCK_SIZE);
            mj->x =mj->x - 1;
            mj->y = mj->y - 1;

            //move block and update block on array
            if(mjHasBlock){
                walkable[mjPrevY][mjPrevX]->sprite->moveBy(-BLOCK_SIZE, BLOCK_SIZE);
                walkable[mj->y][mj->x] =  walkable[mjPrevY][mjPrevX];
                walkable[mjPrevY][mjPrevX] = NULL;

            }
        }
        //going up
        else if(walkable[mj->y-1][mj->x-1] != NULL && walkable[mj->y][mj->x-1] == NULL){
            //move block and update block on array
            if(mjHasBlock && walkable[mj->y+1][mj->x-1] == NULL){
                mj->sprite->moveBy(-BLOCK_SIZE, -BLOCK_SIZE);
                mj->x =mj->x - 1;
                mj->y = mj->y + 1;

                walkable[mjPrevY][mjPrevX]->sprite->moveBy(-BLOCK_SIZE, -BLOCK_SIZE);
                walkable[mj->y][mj->x] =  walkable[mjPrevY][mjPrevX];
                walkable[mjPrevY][mjPrevX] = NULL;
            }
            else if(!mjHasBlock){
                mj->sprite->moveBy(-BLOCK_SIZE, -BLOCK_SIZE);
                mj->x =mj->x - 1;
                mj->y = mj->y + 1;
            }
        }
        else if( mj->x != 0 && walkable[mj->y-2][mj->x-1] != NULL && walkable[mj->y-1][mj->x-1] == NULL){

             //move block and update block on array
            if(mjHasBlock && walkable[mj->y][mj->x - 1] == NULL){
                mj->sprite->moveBy(-BLOCK_SIZE,0);
                mj->x = mj->x - 1;

                walkable[mjPrevY][mjPrevX]->sprite->moveBy(-BLOCK_SIZE, 0);
                walkable[mj->y][mj->x] =  walkable[mjPrevY][mjPrevX];
                walkable[mjPrevY][mjPrevX] = NULL;
            }
            else if(!mjHasBlock){
                mj->sprite->moveBy(-BLOCK_SIZE,0);
                mj->x = mj->x - 1;
            }
        }
    }
    //right
    else if(direction > 0 && prevFacing == facing){
        //going up
        if(walkable[mj->y-1][mj->x+1] != NULL && walkable[mj->y][mj->x+1] == NULL){

            if(mjHasBlock && walkable[mj->y+1][mj->x+1] == NULL){
                mj->sprite->moveBy(BLOCK_SIZE, -BLOCK_SIZE);
                mj->x = mj->x + 1;
                mj->y = mj->y + 1;

                walkable[mjPrevY][mjPrevX]->sprite->moveBy(BLOCK_SIZE, -BLOCK_SIZE);
                walkable[mj->y][mj->x] =  walkable[mjPrevY][mjPrevX];
                walkable[mjPrevY][mjPrevX] = NULL;
            }
            //move block and update block on array
            else if(!mjHasBlock){
                mj->sprite->moveBy(BLOCK_SIZE, -BLOCK_SIZE);
                mj->x = mj->x + 1;
                mj->y = mj->y + 1;
            }
        }
        //going down
        else if(walkable[mj->y-1][mj->x+1] == NULL && walkable[mj->y-2][mj->x+1] == NULL && walkable[mj->y-3][mj->x+1] != NULL){
            mj->sprite->moveBy(BLOCK_SIZE, BLOCK_SIZE);
            mj->x = mj->x + 1;
            mj->y = mj->y - 1;

            //move block and update block on array
            if(mjHasBlock){
                walkable[mjPrevY][mjPrevX]->sprite->moveBy(BLOCK_SIZE, BLOCK_SIZE);
                walkable[mj->y][mj->x] =  walkable[mjPrevY][mjPrevX];
                walkable[mjPrevY][mjPrevX] = NULL;

            }
        }
        else if(mj->x != 29 && walkable[mj->y-2][mj->x+1] != NULL && walkable[mj->y-1][mj->x+1] == NULL){
            //move block and update block on array
            if(mjHasBlock && walkable[mj->y][mj->x+1] == NULL){
                mj->sprite->moveBy(BLOCK_SIZE,0);
                mj->x = mj->x + 1;

                walkable[mjPrevY][mjPrevX]->sprite->moveBy(BLOCK_SIZE, 0);
                walkable[mj->y][mj->x] =  walkable[mjPrevY][mjPrevX];
                walkable[mjPrevY][mjPrevX] = NULL;
            }
            else if(!mjHasBlock){
                mj->sprite->moveBy(BLOCK_SIZE,0);
                mj->x = mj->x + 1;
            }
        }
    }

    prevFacing = facing;
    safeToCheckEnemyCollision = true;
}

/*! \brief engine::moveGood
 * moves the good characters
 */
void engine::moveGood(){
    Node *tmp = goodGuys->head;
    while(tmp != 0){
        if(tmp != mj){

            int direction = tmp->movement;
            //left
            if(direction == 0){
                if( tmp->x != 0 && walkable[tmp->y-2][tmp->x-1] != NULL && walkable[tmp->y-1][tmp->x-1] == NULL){
                    tmp->sprite->moveBy(-BLOCK_SIZE,0);
                    tmp->x = tmp->x - 1;
                }
                //at edge, turn right
                else if(tmp->x != 29 && walkable[tmp->y-2][tmp->x+1] != NULL){
                    //animate sprite here
                    tmp->movement = 1;
                }
            }
            //right
            else if(direction == 1){
                if(tmp->x != 29 && walkable[tmp->y-2][tmp->x+1] != NULL && walkable[tmp->y-1][tmp->x+1] == NULL){
                    tmp->sprite->moveBy(BLOCK_SIZE,0);
                    tmp->x = tmp->x + 1;
                }
                //at edge turn left
                else if( tmp->x != 0 && walkable[tmp->y-2][tmp->x-1] != NULL ){
                    //animate sprite here
                    tmp->movement = 0;
                }
            }
        }
        tmp = tmp->next;
    }
}

/*! \brief engine::moveEnemies
 * moves the good characters
 */
void engine::moveEnemies(){
    Node *tmp = enemies->head;
    while(tmp != 0){
        int direction = tmp->movement;
        //left
        if(direction == 0){
            if( tmp->x != 0 && walkable[tmp->y-2][tmp->x-1] != NULL && walkable[tmp->y-1][tmp->x-1] == NULL){
                tmp->sprite->moveBy(-BLOCK_SIZE,0);
                tmp->x = tmp->x - 1;
                safeToCheckEnemyCollision = true;
            }
            //at edge, turn right
            else if(tmp->x != 29 && walkable[tmp->y-2][tmp->x+1] != NULL){

                /*tmp->sprite->moveBy(BLOCK_SIZE,0);
                tmp->x = tmp->x + 1;
                */
                //animate sprite here
                tmp->movement = 1;
            }
        }
        //right
        else if(direction == 1){
            if(tmp->x != 29 && walkable[tmp->y-2][tmp->x+1] != NULL && walkable[tmp->y-1][tmp->x+1] == NULL){
                tmp->sprite->moveBy(BLOCK_SIZE,0);
                tmp->x = tmp->x + 1;
                safeToCheckEnemyCollision = true;
            }
            //at edge turn left
            else if( tmp->x != 0 && walkable[tmp->y-2][tmp->x-1] != NULL ){
                /*
                tmp->sprite->moveBy(-BLOCK_SIZE,0);
                tmp->x = tmp->x - 1;
                */
                //animate sprite here
                tmp->movement = 0;
            }
        }
        tmp = tmp->next;
    }
}

/*! \brief engine::getBlock
 * allows MJ to pick up blocks below her (when she is facing front)
 * or on the right or left of her, depending on which direction she
 * is facing
 */
void engine::getBlock(){
    int y = mj->y;
    int x = mj->x + facing;
    if(facing ==0)
        y = y - 1;


    //checks to see if the block that mj is facing is of type MBLOCK
    if(walkable[y-1][x]!= 0 && walkable[y-1][x]->blockType.compare("MBLOCK") == 0){
        int mjX = mj->x;
        int mjY = mj->y;

        if(facing == 0){
            walkable[y-1][x]->sprite->moveBy(0, -BLOCK_SIZE);
            walkable[mjY-1][mjX] = walkable[y-1][x];
            walkable[y-1][x] = 0;

            mj->sprite->moveBy(0, BLOCK_SIZE);
            mj->y = mj->y-1;
        }
        else{
            walkable[y-1][x]->sprite->moveBy((mjX - x) * 30, -(mjY -y+1) * 30);
            walkable[mjY][mjX] = walkable[y-1][x];
            walkable[y-1][x] = 0;
        }
        mjHasBlock = true;
    }
}

/*! \brief engine::dropBlock()
 * using mj's position we find the position of the block she is carrying
 * and check to see if the place where the player wants to drop it is a legal
 * place, ie there is no block there. If block lands on an enemy, they die (disappear)
 */
void engine::dropBlock(){

    //has to face left or right
    if(facing == 0)
        return;

    //get x y for block to be used in the walkable array
    int blockY = mj->y;
    int blockX = mj->x;

    //place block
    Node *ptr;
    if(walkable[blockY][blockX + facing] == NULL){
        walkable[blockY][blockX]->sprite->moveBy(facing * 30, 0);
        walkable[blockY][blockX]->x = blockX + facing;
        walkable[blockY][blockX]->y = blockY + 1;

        walkable[blockY][blockX + facing] = walkable[blockY][blockX];
        walkable[blockY][blockX] = NULL;
        ptr = walkable[blockY][blockX + facing];

        //move block down if possible
        Node *tmp = walkable[blockY - 1][blockX + facing];
        int count = 0;
        while(tmp == NULL){
            walkable[blockY - count][blockX + facing]->sprite->moveBy(0, BLOCK_SIZE);
            walkable[blockY - count][blockX + facing]->y = walkable[blockY - count][blockX + facing]->y - 1;

            walkable[blockY - count - 1][blockX + facing] = walkable[blockY - count][blockX + facing];
            walkable[blockY - count][blockX + facing] = NULL;

            ptr = walkable[blockY - count - 1][blockX + facing];

            count ++;
            tmp = walkable[blockY - 1 - count][blockX + facing];
        }

        //check to see if enemy got crushed
        tmp = enemies->head;
        while(tmp != 0){
            if((ptr->x == tmp->x) && (ptr->y == tmp->y)){
                //enemy got crushed remove it and play sound fx
                player->setMedia(QUrl::fromLocalFile(QFileInfo("sounds/squish.wav").absoluteFilePath()));
                player->setVolume(60);
                player->play();
                enemies->remove(tmp);
            }
            tmp = tmp->next;
        }
        //update mj block status
        mjHasBlock = false;
    }

}

/*! \brief engine::checkCollision()
 *  checks to see if MJ collides with any of the people
 *  if she collides with a good guy, she gets the item they are holding
 *  if she collides with an enemy, she loses a life, until she runs out
 *  when she runs out the game resets to the beginning of that level
 */
void engine::checkCollisions(){
    //check to see if mj collides with a good guy
    Node *tmp = goodGuys->head;
    while(tmp != NULL){
        if(tmp->blockType.compare(QString("MJ")) != 0 ){
            if((tmp->x == mj->x) && (tmp->y == mj->y) && (tmp->hasObj)) {

                //draw object on screen
                goodObj[curItems] = new QGraphicsRectWidget(QPixmap(tmp->goodObj), BLOCK_SIZE, BLOCK_SIZE);
                MoveBlock(goodObj[curItems], uiScene, curItems, 20);
                uiScene->addItem(goodObj[curItems]);

                //play sound fx
                player->setMedia(QUrl::fromLocalFile(QFileInfo("sounds/chime.wav").absoluteFilePath() ));
                player->setVolume(70);
                player->play();

                tmp->hasObj = false;
                itemCount --;
                curItems ++;
            }
        }
        tmp = tmp->next;
    }

    //collides with enemies
    tmp = enemies->head;
    while(tmp != NULL){
        if( (mj->x == tmp->x) && (mj->y == tmp->y) && safeToCheckEnemyCollision ){
            life --;
            if(life >=0){
                delete hearts[life];
                hearts[life] = NULL;
            }
            if(life <= 0){
                //remove everything that is drawned and reload the level
                QMessageBox msgBox;
                msgBox.setText("Your lives are gone. Restarting level");
                msgBox.exec();

                reset(parsley->curLevel);
            }

            safeToCheckEnemyCollision = false;
        }
        tmp = tmp->next;
    }
}
