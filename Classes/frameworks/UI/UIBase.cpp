//
// Created by CT on 1/15/16.
//

#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "UIBase.h"

USING_NS_TIMELINE;
using namespace std;

bool UIBase::init() {
    if (Node::init())
    {
        auto node = CSLoader::createNode(_res, [&](Ref *_node) {
            CCLOG("Nodename is %s",dynamic_cast<Node *>(_node)->getName().c_str());
            auto varNode = dynamic_cast<Node *>(_node);
            CCASSERT(varNode != nullptr, "ref is no a _node!!");
            _vars.insert(make_pair(varNode->getName().c_str(), varNode));
        });
        this->addChild(node);
    }
    return true;
}

UIBase *UIBase::create() {
    auto ret = new UIBase();
    if (ret && ret->init())
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}


