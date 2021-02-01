//
//  GroupSelectView.h
//  iPet
//
//  Created by KimSteve on 2017. 6. 23..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef GroupSelectView_h
#define GroupSelectView_h

#include "../Base/SMTableView.h"
#include "../Util/ImageItemInfo.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
class OnGroupSelectListener;

class GroupSelectView : public SMView, public OnClickListener
{
public:
    static GroupSelectView * create(const std::vector<ImageGroupInfo>& groups);
    
    void setOnGroupSelectListener(OnGroupSelectListener* l);
    
    void onImageGroupThumbResult(const std::vector<ImageGroupInfo>& groups);
    
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch * touch, const cocos2d::Vec2 * point, MotionEvent* event) override;
    
protected:
    GroupSelectView();
    virtual ~GroupSelectView();
    
    bool initWithGroupInfo(const std::vector<ImageGroupInfo>& groups);
    
    cocos2d::Node * cellForRowAtIndexPath(const IndexPath & indexPath);
    
    int numberOfRowsInSection(int section);
    
    virtual void onClick(SMView * view) override;
    
private:
    SMTableView * _tableView;
    
    std::vector<ImageGroupInfo> _groups;
    
    OnGroupSelectListener * _listener;
};
#else
class ImageFetcherHost;
class OnGroupSelectListener;

class GroupSelectView : public SMView, public OnClickListener
{
public:
    static GroupSelectView * create(ImageFetcherHost* fetcher);
    
    void setOnGroupSelectListener(OnGroupSelectListener* l);
    
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch * touch, const cocos2d::Vec2 * point, MotionEvent* event) override;
    
protected:
    GroupSelectView();
    virtual ~GroupSelectView();
    
    bool initWithGroupInfo(ImageFetcherHost* fetcher);
    
    cocos2d::Node * cellForRowAtIndexPath(const IndexPath & indexPath);
    
    int numberOfRowsInSection(int section);
    
    virtual void onClick(SMView * view) override;
    
private:
    SMTableView * _tableView;
    
    ImageFetcherHost* _fetcher;
    
    OnGroupSelectListener * _listener;
};
#endif

#endif /* GroupSelectView_h */
