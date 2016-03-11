//
// Created by CT on 1/15/16.
//

#ifndef MYGAME_UIBASE_H
#define MYGAME_UIBASE_H

#include "cocos2d.h"
#include <map>

USING_NS_CC;

class UIBase: public Node {
public:
    UIBase(std::string res = "res/MainScene.csb"):_res(res) {}
    ~UIBase() {}
    
    static UIBase* create();

    virtual bool init() override;
    template<typename valueType>
    valueType getNode(std::string &name) {
        return static_cast<valueType>(_vars.find(name));
    }
private:
    std::string _res;
    std::map<std::string, cocos2d::Node*> _vars;
   };


#endif //MYGAME_UIBASE_H
