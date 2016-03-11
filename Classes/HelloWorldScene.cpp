//#include <frameworks/UI/UIBase.h>
#include "UIBase.h"
#include "HelloWorldScene.h"
#include "xqwlight.hpp"

USING_NS_CC;
const std::string chessResFrefix = "res/images/";
const std::string PIECE_NAME[24] = {
                                    "oo", "", "", "", "", "", "", "",
                                    "rk", "ra", "rb", "rn", "rr", "rc", "rp", "",
                                    "bk", "ba", "bb", "bn", "br", "bc", "bp", "",
};

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label

    auto label = Label::createWithTTF("XQWLIGHT", "fonts/Marker Felt.ttf", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");

    auto center = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);
    // position the sprite on the center of the screen
    sprite->setPosition(center);

    // add the sprite as a child to this layer
    this->addChild(sprite, 0);
    
    // 初始化全局变量
    srand((DWORD) time(NULL));
    InitZobrist();
    LoadBook();
    Xqwl.bFlipped = false;
    Startup();
    
    this->DrawBoard();
    
    return true;
}

void HelloWorld::DrawBoard()
{
    auto winSize = Director::getInstance()->getWinSize();
    auto bg = Sprite::create("board.jpg");
    bg->setPosition(Vec2(winSize.width/2, winSize.height/2));
    this->addChild(bg);
    bool bFlipped = false;
    
    int x, y, xx, yy, sq, pc;
    for (x = FILE_LEFT; x <= FILE_RIGHT; x ++) {
        for (y = RANK_TOP; y <= RANK_BOTTOM; y ++) {
            if (bFlipped) {
                xx = BOARD_EDGE + (FILE_FLIP(x) - FILE_LEFT) * SQUARE_SIZE;
                yy = BOARD_EDGE + (RANK_FLIP(y) - RANK_TOP) * SQUARE_SIZE;
            } else {
                xx = BOARD_EDGE + (x - FILE_LEFT) * SQUARE_SIZE;
                yy = BOARD_EDGE + (y - RANK_TOP) * SQUARE_SIZE;
            }
            sq = COORD_XY(x, y);
            pc = pos.ucpcSquares[sq];
            CCLOG("sq:%d, pc:%d", sq, pc);
//            CCLOG("x-y: %d-%d", xx, yy);
            if (pc != 0) {
                auto sp = Sprite::create(PIECE_NAME[pc] + ".png");
                bg->addChild(sp);
                sp->setPosition(Vec2(xx, yy));
            }
            if (sq == Xqwl.sqSelected || sq == SRC(Xqwl.mvLast) || sq == DST(Xqwl.mvLast)) {
                
            }
        }
    }
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
