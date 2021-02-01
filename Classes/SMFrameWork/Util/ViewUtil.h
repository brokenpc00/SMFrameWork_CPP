//
//  ViewUtil.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#ifndef ViewUtil_h
#define ViewUtil_h

#include <2d/CCNode.h>
#include <2d/CCLabel.h>
#include <2d/CCFont.h>
#include <2d/CCFontAtlasCache.h>
#include <deprecated/CCString.h>
#include <base/ccUTF8.h>
#include <string>
#include <cmath>
#include <math.h>
#include "DownloadTask.h"

#define roundT(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

#define MAKE_COLOR4Frgba(rgba) (cocos2d::Color4F((((0xFF000000&(rgba))>>24)/255.0f),(((0x00FF0000&(rgba))>>16)/255.0f),(((0x0000FF00&(rgba))>>8)/255.0f), ((0x000000FF&(rgba))/255.0f)))
#define MAKE_COLOR4F(rgb, a) (cocos2d::Color4F((((0x00FF0000&(rgb))>>16)/255.0f),(((0x0000FF00&(rgb))>>8)/255.0f),((0x000000FF&(rgb))/255.0f), (a)))
#define MAKE_COLOR4F_M(rgb, mult, a) (cocos2d::Color4F((((0x00FF0000&(rgb))>>16)*mult/255.0f),(((0x0000FF00&(rgb))>>8)*mult/255.0f),((0x000000FF&(rgb))*mult/255.0f), (a)))
#define MAKE_COLOR3B(rgb) (cocos2d::Color3B(((0x00FF0000&(rgb))>>16),((0x0000FF00&(rgb))>>8),(0x000000FF&(rgb))))
#define MAKE_COLOR4B(argb) (cocos2d::Color4B(((0x00FF0000&(argb))>>16),((0x0000FF00&(argb))>>8),(0x000000FF&(argb)),((0xFF000000&(argb))>>24)))

#define MAKE_COLOR4D(r, g, b, a) (cocos2d::Color4F(r/255.0f, g/255.0f, b/255.0f, a))


class ViewUtil {
    
public:
    // for image size and rect
    static cocos2d::Rect frameForImageInTargetFrame(cocos2d::Size imageSize, cocos2d::Rect targetFrame) {
        float imageRatio = imageSize.width / imageSize.height;
        float targetRatio = targetFrame.size.width / targetFrame.size.height;
        
        cocos2d::Rect fittedRect = targetFrame;
        if (imageRatio > targetRatio) {
            fittedRect.size.height = targetFrame.size.width * imageSize.height / imageSize.width;
            fittedRect.origin.y += (targetFrame.size.height - fittedRect.size.height) / 2.0f;
        } else {
            fittedRect.size.width = targetFrame.size.height * imageSize.width / imageSize.height;
            fittedRect.origin.x += (targetFrame.size.width - fittedRect.size.width) / 2.0f;
        }
        return cocos2d::Rect(roundT(fittedRect.origin.x), roundT(fittedRect.origin.y), roundT(fittedRect.size.width), roundT(fittedRect.size.height));
    }
    
    static void adoptionTo(cocos2d::Node* targetChild, cocos2d::Node* newParent, int localZOrder = 0) {
        CC_ASSERT( targetChild != nullptr && targetChild->getParent() != nullptr && newParent != nullptr );
        
        targetChild->retain();
        targetChild->getParent()->removeChild(targetChild, false);
        newParent->addChild(targetChild, localZOrder);
        targetChild->release();
    }
    
    static inline cocos2d::Vec4 colorToVec4(const cocos2d::Color4B& color) {
        return cocos2d::Vec4(color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a/255.0f);
    }
    
    static inline cocos2d::Vec4 colorToVec4(const cocos2d::Color3B& color, const GLubyte opacity) {
        return cocos2d::Vec4(color.r/255.0f, color.g/255.0f, color.b/255.0f, opacity/255.0f);
    }
    
    static inline cocos2d::Vec4 colorToVec4(const cocos2d::Color4F& color) {
        return cocos2d::Vec4(color.r, color.g, color.b, color.a);
    }
    
    static inline cocos2d::Color4B vec4ToColor4B(const cocos2d::Vec4& color) {
        return cocos2d::Color4B((GLubyte)(255.0f*color.x), (GLubyte)(255.0f*color.y), (GLubyte)(255.0f*color.z), (GLubyte)(255.0f*color.w));
    }

    static inline cocos2d::Color3B vec4ToColor3B(const cocos2d::Vec4& color) {
        return cocos2d::Color3B((GLubyte)(255.0f*color.x), (GLubyte)(255.0f*color.y), (GLubyte)(255.0f*color.z));
    }
    
    static inline cocos2d::Color4F vec4ToColor4F(const cocos2d::Vec4& color) {
        return cocos2d::Color4F(color.x, color.y, color.z, color.w);
    }
    
    static inline GLubyte vec4ToOpacity(const cocos2d::Vec4& color) {
        return (GLubyte)(255.0f*color.w);
    }
    
    static inline cocos2d::Color3B color4FToColor3B(const cocos2d::Color4F& color) {
        return cocos2d::Color3B((GLubyte)(255.0f*color.r), (GLubyte)(255.0f*color.g), (GLubyte)(255.0f*color.b));
    }
    
    static inline cocos2d::Color4B color4FToColor4B(const cocos2d::Color4F& color) {
        return cocos2d::Color4B((GLubyte)(255.0f*color.r), (GLubyte)(255.0f*color.g), (GLubyte)(255.0f*color.b), (GLubyte)(255.0f*color.a));
    }
    
    static void shortenLabel(cocos2d::Label* label, int limitWidth) {
        std::string receivers = label->getString();
        
        std::string curString = receivers;
        bool bCut = false;
        while (label->getContentSize().width>limitWidth) {
            curString = curString.substr(0, curString.length()-1);
            label->setString(curString);
            bCut = true;
        }
        
        if (bCut) {
            curString += "...";
            label->setString(curString);
        }
    }
    
    /**
     * http://lekdw.blogspot.kr/2014/11/cocos2d-x-label-ellipse.html
     */
    static void shortenLabelTTFString(cocos2d::Label* label, int limitWidth) {
        std::u32string utf32EllipseString;
        cocos2d::StringUtils::UTF8ToUTF32("...", utf32EllipseString);
        
        std::u32string utf32String;
        cocos2d::StringUtils::UTF8ToUTF32(label->getString(), utf32String);
        
        auto ttfConfig = label->getTTFConfig();
        cocos2d::FontAtlas* ttfAtlas = cocos2d::FontAtlasCache::getFontAtlasTTF(&ttfConfig);
        ttfAtlas->prepareLetterDefinitions(utf32EllipseString);
        
        int letterCount = 0;
        auto font = ttfAtlas->getFont();
        int* kerning = font->getHorizontalKerningForTextUTF32(utf32EllipseString, letterCount);
        
        int ellipseWidth = 0;
        cocos2d::FontLetterDefinition letterDef = {0};
        
        for (size_t i = 0; i < utf32EllipseString.length(); i++) {
            ttfAtlas->getLetterDefinitionForChar(utf32EllipseString[i], letterDef);
            ellipseWidth += letterDef.xAdvance + kerning[i] + label->getAdditionalKerning();
        }
        
        if (kerning != nullptr)
            delete[] kerning;
        
        if (ellipseWidth > limitWidth) {
            label->setString("...");
            return;
        }
        
        letterCount = 0;
        kerning = ttfAtlas->getFont()->getHorizontalKerningForTextUTF32(
                                                                        utf32String, letterCount);
        
        std::string result;
        std::u16string utf16Result;
        int width = ellipseWidth;
        
        for (size_t i = 0; i < utf32String.length(); i++) {
            ttfAtlas->getLetterDefinitionForChar(utf32String[i], letterDef);
            
            if (width + letterDef.xAdvance > limitWidth) {
                cocos2d::StringUtils::UTF16ToUTF8(utf16Result, result);
                label->setString(result + "...");
                break;
            }
            
            width += letterDef.xAdvance + kerning[i] + label->getAdditionalKerning();
            
            utf16Result += utf32String[i];
        }
        
        if (kerning != nullptr)
            delete[] kerning;
    }
    
    static float getDecelateInterpolation(float t) {
        return (float)(1.0 - (1.0 - t) * (1.0 - t));
    }
    
    static float getDecelateInterpolation(float t, float fractor) {
        return (float)(1.0 - std::pow((1.0 - t), 2 * fractor));
    }
    
    static inline float interpolation(float from, float to, float t) {
        return from + (to - from) * t;
    }
    
    static cocos2d::Color4F interpolateColor4F(const cocos2d::Color4F& from, const cocos2d::Color4F& to, float t) {
        float a = interpolation(from.a, to.a, t);
        float r = interpolation(from.r, to.r, t);
        float g = interpolation(from.g, to.g, t);
        float b = interpolation(from.b, to.b, t);
        
        return cocos2d::Color4F(r, g, b, a);
    }
    
    static cocos2d::Color3B interpolateColor3B(const cocos2d::Color3B& from, const cocos2d::Color3B& to, float t) {
        GLubyte r = (GLubyte)interpolation(from.r, to.r, t);
        GLubyte g = (GLubyte)interpolation(from.g, to.g, t);
        GLubyte b = (GLubyte)interpolation(from.b, to.b, t);
        
        return cocos2d::Color3B(r, g, b);
    }
    
    static cocos2d::Color4F interpolateColor4F(uint32_t from, uint32_t to, float t) {
        float a = interpolation((from&0xFF000000)>>24, (to&0xFF000000)>>24, t) / 255.0;
        float r = interpolation((from&0x00FF0000)>>16, (to&0x00FF0000)>>16, t) / 255.0;
        float g = interpolation((from&0x0000FF00)>>8,  (to&0x0000FF00)>>8,  t) / 255.0;
        float b = interpolation((from&0x000000FF),     (to&0x000000FF),     t) / 255.0;
        
        return cocos2d::Color4F(r, g, b, a);
    }
    
    static cocos2d::Color4F interpolateColor4F(uint32_t from, uint32_t to, float t, uint32_t& outValue) {
        float a = interpolation((from&0xFF000000)>>24, (to&0xFF000000)>>24, t) / 255.0;
        float r = interpolation((from&0x00FF0000)>>16, (to&0x00FF0000)>>16, t) / 255.0;
        float g = interpolation((from&0x0000FF00)>>8,  (to&0x0000FF00)>>8,  t) / 255.0;
        float b = interpolation((from&0x000000FF),     (to&0x000000FF),     t) / 255.0;
        
        outValue =  (((uint32_t)(a*0xFF))<<24)|(((uint32_t)(r*0xFF))<<16)|(((uint32_t)(g*0xFF))<<8)|((uint32_t)(b*0xFF));
        
        return cocos2d::Color4F(r, g, b, a);
    }
    
    static cocos2d::Color4F uint32ToColor4F(uint32_t value) {
        float a = ((value&0xFF000000)>>24) / 255.0;
        float r = ((value&0x00FF0000)>>16) / 255.0;
        float g = ((value&0x0000FF00)>>8) / 255.0;
        float b = ((value&0x000000FF)) / 255.0;
        
        return cocos2d::Color4F(r, g, b, a);
    }
    
    static double toRadians(double degrees) {
        return ( degrees * M_PI ) / 180.0;
    }
    
    static double toDegrees(double radians) {
        return ( radians * 180.0 ) / M_PI ;
    }
    
    static int round(float value) {
        return (int)(value+0.5);
    }
    
    static int signum(float value) {
        return value >= 0 ? 1 : -1;
    }
    
    static float shortestAngle(float from, float to) {
        return std::fmod((std::fmod(to-from, 360.0)+540), 360.0)-180;
    }
    
    enum Direction {
        UP = 0, LEFT, DOWN, RIGHT
    };
    
    static Direction getDirection(float dx, float dy) {
        const int VERTICAL_WIDE = 100;
        const int HORIZONTAL_WIDE = (180-VERTICAL_WIDE);
        
        double radians = std::atan2(dy, dx);
        int degrees = (int)toDegrees(radians);
        degrees = (degrees % 360) + (degrees < 0 ? 360 : 0); // normalize
        
        int a = HORIZONTAL_WIDE/2;
        if (degrees > a && degrees < a + VERTICAL_WIDE) {
            return UP;
        }
        a += VERTICAL_WIDE;
        
        if (degrees > a && degrees < a + HORIZONTAL_WIDE) {
            return LEFT;
        }
        a += HORIZONTAL_WIDE;
        
        if (degrees > a && degrees < a + VERTICAL_WIDE) {
            return DOWN;
        }
        
        return RIGHT;
    }
    
    static cocos2d::Texture2D * makeBitmapCopy(cocos2d::Node *node, const cocos2d::Size &canvasSize, const cocos2d::Vec2 &position, const cocos2d::Vec2 &anchorPoint, const float scaleX, const float scaleY)
    {
        auto old_position = node->getPosition();
        auto old_anchor = node->getAnchorPoint();
        auto old_scaleX = node->getScaleX();
        auto old_scaleY = node->getScaleY();
        auto old_rotate = node->getRotation();
        auto old_opacity = node->getOpacity();
        
        auto rt = cocos2d::RenderTexture::create(canvasSize.width, canvasSize.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
        auto renderer = cocos2d::Director::getInstance()->getRenderer();
        
        rt->beginWithClear(0, 0, 0, 0);
        // begin
        node->setPosition(position);
        node->setAnchorPoint(anchorPoint);
        node->setScale(scaleX, scaleY);
        node->setRotation(0);
        node->setOpacity(0xff);
        
        // capture를 위해 한번 뿌림
        node->visit(renderer, cocos2d::Mat4::IDENTITY, 0);
        // end
        rt->end();
        // capture!!!!
        renderer->render();
        
        
        // restore
        node->setPosition(old_position);
        node->setAnchorPoint(old_anchor);
        node->setScale(old_scaleX, old_scaleY);
        node->setRotation(old_rotate);
        node->setOpacity(old_opacity);
        
        // error check
        auto errorCode = glGetError();
        if (errorCode != GL_NO_ERROR) {
            // error capture
            CCLOG("[[[[[ Catprue Error : 0x%x", (int)errorCode);
            return nullptr;
        }
        
        return rt->getSprite()->getTexture();
    }
    
};



#endif /* ViewUtil_h */
