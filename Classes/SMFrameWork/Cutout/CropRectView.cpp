//
//  CropRectView.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 28..
//

#include "CropRectView.h"
#include "../Base/SMButton.h"
#include "../Base/ShaderNode.h"
#include "../Base/ViewAction.h"
#include "../Const/SMFontColor.h"

#define THUMB_RADIUS    (30)
#define THUMB_PADDING   (30)
#define EDGE_SIZE       (60)

#define MIN_CROP_SIZE   (150)

#define TAG_FOCUS_PANEL     (1)
#define TAG_CORNER_MOVER    (2)
#define TAG_EDGE_MOVER      (TAG_CORNER_MOVER+4)


#define ACTION_TAG_SHOW (100)

#define DIM_OPACITY_EDIT    (0.5)
#define DIM_OPACITY_IDLE    (0.9)


class CropRectView::FocusView : public SMView
{
public:
    static FocusView * create();
    
    virtual void setContentSize(const cocos2d::Size& contentSize) override;
    
    void showGrid(bool show, float duration, float delay);
    
    void setPressedEdge(int edgeIndex);
    
protected:
    virtual bool init() override;
    
private:
    enum Type {
        TOP,
        BOTTOM,
        LEFT,
        RIGHT,
        HOR_1,
        HOR_2,
        VER_1,
        VER_2
    };
    
    void setLine(Type type, float x, float y, float length, float thickness);
    
private:
    cocos2d::Node* _gridPanel;
    
    ShapeSolidRect* _line[8];
    
    ShapeSolidRect* _shadow[4];
    
    int _pressedEdgeindex;
};


CropRectView* CropRectView::create(cocos2d::Sprite *sprite, const cocos2d::Rect &boundRect)
{
    auto view = new (std::nothrow)CropRectView();
    if (view) {
        if (view->init(sprite, boundRect)) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    
    return view;
}

bool CropRectView::init(cocos2d::Sprite *sprite, const cocos2d::Rect &boundRect)
{
    BaseCropView::init(sprite, boundRect);
    
    _cropRect = boundRect;
    
    _focusView = FocusView::create();
    _focusView->setTag(TAG_FOCUS_PANEL);
    _focusView->setOnTouchListener(this);
    _uiLayer->addChild(_focusView);
    
    // Edge 이동 뷰
    for (int i=0; i<4; i++) {
        auto view = SMView::create(TAG_EDGE_MOVER+i);
        view->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        view->setOnTouchListener(this);
        _edgeMover[i] = view;
        _uiLayer->addChild(_edgeMover[i]);
    }
    
    // corner 이동 버튼
    for (int i=0; i<4; i++) {
        auto btn = SMButton::create(TAG_CORNER_MOVER+i, SMButton::Style::SOLID_CIRCLE);
        btn->setContentSize(cocos2d::Size(2*(THUMB_RADIUS+THUMB_PADDING), 2*(THUMB_RADIUS+THUMB_PADDING)));
        btn->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        btn->setPadding(THUMB_PADDING);
        btn->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        btn->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xadafb3, 1.0f));
        
        auto shadow = ShapeSolidCircle::create();
        btn->setBackgroundNode(shadow);
        // 약간 크게
        shadow->setContentSize(cocos2d::Size(2*THUMB_RADIUS*1.125, 2*THUMB_RADIUS*1.125));
        shadow->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        shadow->setAntiAliasWidth(THUMB_RADIUS*0.3);
        shadow->setPosition(THUMB_RADIUS+THUMB_PADDING, THUMB_RADIUS+THUMB_PADDING-5);
        
        btn->setBackgroundColor4F(MAKE_COLOR4F(0, 0.15));
        btn->setOnTouchListener(this);
        
        _cornerMover[i] = btn;
        _uiLayer->addChild(_cornerMover[i]);
    }
    
    updateCropBound(_boundRect, true);
    
    return true;
}

cocos2d::Sprite* CropRectView::cropSpriteInternal(cocos2d::Rect &outRect)
{
    return _maskSprite->cropSprite(_textureRect, outRect);
}

bool CropRectView::cropDataInternal(uint8_t **outPixelData, uint8_t **outMaskData, cocos2d::Rect &outRect)
{
    return _maskSprite->cropData(_textureRect, outPixelData, outMaskData, outRect);
}

void CropRectView::updateCropBound(const cocos2d::Rect &newRect, bool force, int edgeIndex)
{
    if (!force & newRect.equals(_cropRect)) {
        return;
    }
    
    _cropRect = newRect;
    
    // forcus view
    _focusView->setPosition(_cropRect.origin);
    _focusView->setPressedEdge(edgeIndex);
    _focusView->setContentSize(_cropRect.size);
    
    // corner mover
    _cornerMover[0]->setPosition(_cropRect.getMinX(), _cropRect.getMaxY()); // LT
    _cornerMover[1]->setPosition(_cropRect.getMaxX(), _cropRect.getMaxY()); // RT
    _cornerMover[2]->setPosition(_cropRect.getMinX(), _cropRect.getMinY()); // LB
    _cornerMover[3]->setPosition(_cropRect.getMaxX(), _cropRect.getMinY()); // RB

    // edge mover
    _edgeMover[0]->setContentSize(cocos2d::Size(_cropRect.size.width, EDGE_SIZE));  // Top
    _edgeMover[0]->setPosition(_cropRect.getMidX(), _cropRect.getMaxY());
    _edgeMover[1]->setContentSize(cocos2d::Size(_cropRect.size.width, EDGE_SIZE));  // Bottom
    _edgeMover[1]->setPosition(_cropRect.getMidX(), _cropRect.getMinY());
    _edgeMover[2]->setContentSize(cocos2d::Size(EDGE_SIZE, _cropRect.size.height)); // Left
    _edgeMover[2]->setPosition(_cropRect.getMinX(), _cropRect.getMidY());
    _edgeMover[3]->setContentSize(cocos2d::Size(EDGE_SIZE, _cropRect.size.height)); // Right
    _edgeMover[3]->setPosition(_cropRect.getMaxX(), _cropRect.getMidY());

    // scale
    auto scale = _textureSize.width / _boundRect.size.width;
    _textureRect.origin.x =  (_cropRect.getMinX()-_boundRect.getMinX())*scale;
    _textureRect.origin.y =  (_boundRect.getMaxY()-_cropRect.getMaxY())*scale;
    _textureRect.size = _cropRect.size*scale;

    _maskSprite->setTextureRect(_textureRect, false, _textureSize);
    _maskSprite->setPosition(_cropRect.getMidX(), _cropRect.getMidY());
}

void CropRectView::showGrid(bool show, float duration, float delay)
{
    setDimOpacity(show?DIM_OPACITY_EDIT:DIM_OPACITY_IDLE, duration, delay);
    _focusView->showGrid(show, duration, delay);
}

int CropRectView::onTouch(SMView* view, const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event)
{
    int tag = view->getTag();
    auto canvasPt = convertToNodeSpace(view->convertToWorldSpace(*point));
    
    
    switch (action) {
        case MotionEvent::ACTION_DOWN:
        {
            // 누르면 그리드 보이고
            showGrid(true, 0.5, 0);
        }
            break;
        case MotionEvent::ACTION_UP:
        case MotionEvent::ACTION_CANCEL:
        {
            // 떼면 그리도 안 보이고
            showGrid(false, 0.3, 1.0);
        }
            break;
    }
    
    if (tag == TAG_FOCUS_PANEL) {
        // 가운데 포커스 뷰
        auto size = _focusView->getContentSize();
        switch (action) {
            case MotionEvent::ACTION_DOWN:
            {
                _grabPt = *point;
            }
                break;
            case MotionEvent::ACTION_MOVE:
            {
                // 처음 누른 좌표에서 얼마나 움직였나
                auto pt = canvasPt - _grabPt;
                
                pt.x = std::min(std::max(pt.x, _boundRect.getMinX()), _boundRect.getMaxX());
                pt.y = std::min(std::max(pt.y, _boundRect.getMinY()), _boundRect.getMaxY());
                if (pt.x+size.width > _boundRect.getMaxX()) {
                    pt.x = _boundRect.getMaxX()-size.width;
                }
                if (pt.y+size.height > _boundRect.getMaxY()) {
                    pt.y = _boundRect.getMaxY()-size.height;
                }
                
                cocos2d::Rect newRect(pt, size);
                // 그리드 뷰를 움직여 준다.
                updateCropBound(newRect);
            }
                
                return TOUCH_TRUE;
        }
    } else if (tag >= TAG_CORNER_MOVER && tag < TAG_CORNER_MOVER+4) {
        // 코너 이동 버튼
        tag -= TAG_CORNER_MOVER;
        
        switch (action) {
            case MotionEvent::ACTION_DOWN:
            {
                _grabPt = *point - cocos2d::Vec2(THUMB_RADIUS+THUMB_PADDING, THUMB_RADIUS+THUMB_PADDING);
            }
                break;
            case MotionEvent::ACTION_MOVE:
            {
                auto pt = canvasPt - _grabPt;
                pt.x = std::min(std::max(pt.x, _boundRect.getMinX()), _boundRect.getMaxX());
                pt.y = std::min(std::max(pt.y, _boundRect.getMinY()), _boundRect.getMaxY());
                
                cocos2d::Rect newRect(_cropRect);
                if (tag%2 == 0) {
                    // 왼쪽 점들
                    if (pt.x > newRect.getMaxX() - MIN_CROP_SIZE) {
                        pt.x = newRect.getMaxX() - MIN_CROP_SIZE;
                    }
                    newRect.origin.x = pt.x;
                    newRect.size.width = _cropRect.getMaxX() - pt.x;
                } else {
                    // 오른쪽 점들
                    if (pt.x < newRect.getMinX() + MIN_CROP_SIZE) {
                        pt.x = newRect.getMinX() + MIN_CROP_SIZE;
                    }
                    newRect.size.width = pt.x - newRect.origin.x;
                }
                
                if (tag/2 == 0) {
                    // 위쪽 점들
                    if (pt.y < newRect.getMinY() + MIN_CROP_SIZE) {
                        pt.y = newRect.getMinY() + MIN_CROP_SIZE;
                    }
                    newRect.size.height = pt.y - newRect.origin.y;
                } else {
                    // 아래쪽 점들
                    if (pt.y > newRect.getMaxY() - MIN_CROP_SIZE) {
                        pt.y = newRect.getMaxY() - MIN_CROP_SIZE;
                    }
                    newRect.origin.y = pt.y;
                    newRect.size.height = _cropRect.getMaxY() - pt.y;
                }
                
                updateCropBound(newRect);
            }
                
                return TOUCH_TRUE;
        }
    } else if (tag >= TAG_EDGE_MOVER && tag < TAG_EDGE_MOVER+4) {
        // 에지 이동 뷰
        tag -= TAG_EDGE_MOVER;
        
        switch (action) {
            case MotionEvent::ACTION_DOWN:
            {
                if (tag == 0 || tag == 1) {
                    _grabPt = *point - cocos2d::Vec2(0, 0.5*EDGE_SIZE);
                } else {
                    _grabPt = *point - cocos2d::Vec2(0.5*EDGE_SIZE, 0);
                }
                
                updateCropBound(_cropRect, true, tag);
            }
                break;
            case MotionEvent::ACTION_MOVE:
            {
                auto pt = canvasPt - _grabPt;
                pt.x = std::min(std::max(pt.x, _boundRect.getMinX()), _boundRect.getMaxX());
                pt.y = std::min(std::max(pt.y, _boundRect.getMinY()), _boundRect.getMaxY());
                
                cocos2d::Rect newRect(_cropRect);
                switch (tag) {
                    case 0:
                        // 위
                        if (pt.y < newRect.getMinY() + MIN_CROP_SIZE) {
                            pt.y = newRect.getMinY() + MIN_CROP_SIZE;
                        }
                        newRect.size.height = pt.y - newRect.origin.y;
                        break;
                    case 1:
                        // 아래
                        if (pt.y > newRect.getMaxY() - MIN_CROP_SIZE) {
                            pt.y = newRect.getMaxY() - MIN_CROP_SIZE;
                        }
                        newRect.origin.y = pt.y;
                        newRect.size.height = _cropRect.getMaxY() - pt.y;
                        break;
                    case 2:
                        // 왼쪽
                        if (pt.x > newRect.getMaxX() - MIN_CROP_SIZE) {
                            pt.x = newRect.getMaxX() - MIN_CROP_SIZE;
                        }
                        newRect.origin.x = pt.x;
                        newRect.size.width = _cropRect.getMaxX() - pt.x;
                        break;
                    case 3:
                        // 오른쪽
                        if (pt.x < newRect.getMinX() + MIN_CROP_SIZE) {
                            pt.x = newRect.getMinX() + MIN_CROP_SIZE;
                        }
                        newRect.size.width = pt.x - newRect.origin.x;
                        break;
                    default:
                        break;
                        
                }
                
                updateCropBound(newRect, false, tag);
            }
                return TOUCH_TRUE;
            case MotionEvent::ACTION_UP:
            case MotionEvent::ACTION_CANCEL:
                updateCropBound(_cropRect, true, -1);
                break;
        }
    }

    return TOUCH_FALSE;
}



// FocusView
CropRectView::FocusView* CropRectView::FocusView::create()
{
    auto view = new (std::nothrow)FocusView();
    
    if (view!=nullptr) {
        if (view->init()) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    
    return view;
}

bool CropRectView::FocusView::init()
{
    _gridPanel = cocos2d::Node::create();
    _gridPanel->setCascadeOpacityEnabled(true);
    _gridPanel->setOpacity(0);
    addChild(_gridPanel);

    for (int i = 0; i < 4; i++) {
        _shadow[i] = ShapeSolidRect::create();
        _shadow[i]->setColor(MAKE_COLOR3B(0x222222));
        _shadow[i]->setOpacity(50);
        addChild(_shadow[i]);
    }

    for (int i = 0; i < 8; i++) {
        _line[i] = ShapeSolidRect::create();
        _line[i]->setColor(MAKE_COLOR3B(0xffffff));
        if (i < 4) {
            addChild(_line[i]);
        } else {
            _line[i]->setOpacity(200);
            _gridPanel->addChild(_line[i]);
        }
    }
    
    return true;
}

void CropRectView::FocusView::showGrid(bool show, float duration, float delay)
{
    auto a = _gridPanel->getActionByTag(ACTION_TAG_SHOW);
    if (a) {
        _gridPanel->stopAction(a);
    }
    
    auto action = ViewAction::TransformAction::create();
    action->setTag(ACTION_TAG_SHOW);
    action->toAlpha(show?1.0:0.0).setTimeValue(duration, delay);
    _gridPanel->runAction(action);
}

#define BORDER_THICKNESS    (2.0)
#define DIV_THICKNESS       (2.0)
#define SHADOW_OFFSET       (2.0)


void CropRectView::FocusView::setContentSize(const cocos2d::Size& contentSize)
{
    SMView::setContentSize(contentSize);
    
    // border
    setLine(Type::TOP, 0, contentSize.height, contentSize.width, _pressedEdgeindex==0?BORDER_THICKNESS*3:BORDER_THICKNESS);
    setLine(Type::BOTTOM, 0, 0, contentSize.width, _pressedEdgeindex==1?BORDER_THICKNESS*3:BORDER_THICKNESS);
    setLine(Type::LEFT, 0, 0, contentSize.height, _pressedEdgeindex==2?BORDER_THICKNESS*3:BORDER_THICKNESS);
    setLine(Type::RIGHT, contentSize.width, 0, contentSize.height, _pressedEdgeindex==3?BORDER_THICKNESS*3:BORDER_THICKNESS);
    
    // inner line
    setLine(Type::HOR_1, 0, 1.0*contentSize.height/3.0, contentSize.width, DIV_THICKNESS);
    setLine(Type::HOR_2, 0, 2.0*contentSize.height/3.0, contentSize.width, DIV_THICKNESS);
    setLine(Type::VER_1, 1.0*contentSize.width/3.0, 0, contentSize.height, DIV_THICKNESS);
    setLine(Type::VER_2, 2.0*contentSize.width/3.0, 0, contentSize.height, DIV_THICKNESS);
}

void CropRectView::FocusView::setPressedEdge(int edgeIndex)
{
    _pressedEdgeindex = edgeIndex;
}

void CropRectView::FocusView::setLine(Type type, float x1, float y1, float length, float thickness)
{
    int index;
    float x2, y2, sx1, sy1, sx2, sy2;
    
    switch (type) {
        case TOP:
            index = 0;
            x1 = x1-thickness;
            x2 = x1+length+thickness*2;
            y2 = y1+thickness;
            sx1 = x1-SHADOW_OFFSET;
            sx2 = x2+SHADOW_OFFSET;
            sy1 = y1;
            sy2 = y2+SHADOW_OFFSET;
            break;
        case BOTTOM:
            index = 1;
            x1 = x1-thickness;
            x2 = x1+length+thickness*2;
            y2 = y1-thickness;
            sx1 = x1-SHADOW_OFFSET;
            sx2 = x2+SHADOW_OFFSET;
            sy1 = y1;
            sy2 = y2-SHADOW_OFFSET;
            break;
        case LEFT:
            index = 2;
            x2 = x1-thickness;
            y2 = y1+length;
            sx1 = x1;
            sx2 = x2-SHADOW_OFFSET;
            sy1 = y1;
            sy2 = y2;
            break;
        case RIGHT:
            index = 3;
            x2 = x1+thickness;
            y2 = y1+length;
            sx1 = x1;
            sx2 = x2+SHADOW_OFFSET;
            sy1 = y1;
            sy2 = y2;
            break;
        case HOR_1:
            index = 4;
            x2 = x1+length;
            y1 = y1-0.5*thickness;
            y2 = y1+thickness;
            sx1 = x1;
            sx2 = x2;
            sy1 = y1-SHADOW_OFFSET;
            sy2 = y2+SHADOW_OFFSET;
            break;
        case HOR_2:
            index = 5;
            x2 = x1+length;
            y1 = y1-0.5*thickness;
            y2 = y1+thickness;
            sx1 = x1;
            sx2 = x2;
            sy1 = y1-SHADOW_OFFSET;
            sy2 = y2+SHADOW_OFFSET;
            break;
        case VER_1:
            index = 6;
            x1 = x1-0.5*thickness;
            x2 = x1+thickness;
            y2 = y1+length;
            sx1 = x1-SHADOW_OFFSET;
            sx2 = x2+SHADOW_OFFSET;
            sy1 = y1;
            sy2 = y2;
            break;
        case VER_2:
            index = 7;
            x1 = x1-0.5*thickness;
            x2 = x1+thickness;
            y2 = y1+length;
            sx1 = x1-SHADOW_OFFSET;
            sx2 = x2+SHADOW_OFFSET;
            sy1 = y1;
            sy2 = y2;
            break;
    }
    
    
    cocos2d::Rect r1(std::min(x1, x2), std::min(y1, y2), std::abs(x2-x1), std::abs(y2-y1));
    
    _line[index]->setPosition(r1.origin);
    _line[index]->setContentSize(r1.size);
    
    if (index < 4) {
        cocos2d::Rect r2(std::min(sx1, sx2), std::min(sy1, sy2), std::abs(sx2-sx1), std::abs(sy2-sy1));
        _shadow[index]->setPosition(r2.origin);
        _shadow[index]->setContentSize(r2.size);
    }

}

