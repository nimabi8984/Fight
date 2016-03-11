//
//  MainScene.hpp
//  Fight
//
//  Created by CT on 2/22/16.
//
//

#ifndef MainScene_hpp
#define MainScene_hpp

#include <stdio.h>
#include "UIBase.h"

class MainScene: public UIBase {
    
    
public:
    virtual bool init() override;
    void initUI();
};

#endif /* MainScene_hpp */
