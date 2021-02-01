//
//  SMImageEditorListener.h
//  iPet
//
//  Created by KimSteve on 2017. 4. 27..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorListener_h
#define SMImageEditorListener_h

#include <cocos2d.h>

class SMImageEditorListener
{
public:
    virtual void onImageEditResult(cocos2d::Sprite * editImage) = 0;
};


#endif /* SMImageEditorListener_h */
