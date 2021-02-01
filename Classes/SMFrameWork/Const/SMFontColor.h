//
//  SMFontColor.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 14..
//
//

#ifndef SMFontColor_h
#define SMFontColor_h

#include <cocos2d.h>

class SMFontConst
{
public:
    static const char* SystemFontRegular;
    static const char* SystemFontBold;
    static const char* SystemFontLight;
    
    static const char* NotoSansLight;
    static const char* NotoSansRegular;
    static const char* NotoSansMedium;
    static const char* NotoSansBold;

    static const char* MontSerratRegular;
    static const char* MontSerratBold;
    
    static const char* SFProDisplayThin;
    static const char* SFProDisplayLight;
    static const char* SFProDisplayMedium;
    static const char* SFProDisplayBold;
};

class SMColorConst
{
public:
    static const cocos2d::Color4F COLOR_F_WHITE;
    static const cocos2d::Color4F COLOR_F_BLACK;
    static const cocos2d::Color4F COLOR_F_EEEFF1;
    static const cocos2d::Color4F COLOR_F_ADAFB3;
    static const cocos2d::Color4F COLOR_F_DBDCDF;
    static const cocos2d::Color4F COLOR_F_666666;
    static const cocos2d::Color4F COLOR_F_999999;
    static const cocos2d::Color4F COLOR_F_dddddd;
    static const cocos2d::Color4F COLOR_F_333333;
    static const cocos2d::Color4F COLOR_F_222222;
    static const cocos2d::Color4F COLOR_F_e94253;
    static const cocos2d::Color4F COLOR_F_37c267;
    static const cocos2d::Color4F COLOR_F_8b6bff;
    static const cocos2d::Color4F COLOR_F_8768f8;
    static const cocos2d::Color4F COLOR_F_26cec1;
    static const cocos2d::Color4F COLOR_F_eeeeee;
    static const cocos2d::Color4F COLOR_F_cccccc;
    static const cocos2d::Color4F COLOR_F_f3f3f3;

    static const cocos2d::Color3B COLOR_B_WHITE;
    static const cocos2d::Color3B COLOR_B_BLACK;
    static const cocos2d::Color3B COLOR_B_EEEFF1;
    static const cocos2d::Color3B COLOR_B_ADAFB3;
    static const cocos2d::Color3B COLOR_B_DBDCDF;
    static const cocos2d::Color3B COLOR_B_666666;
    static const cocos2d::Color3B COLOR_B_999999;
    static const cocos2d::Color3B COLOR_B_dddddd;
    static const cocos2d::Color3B COLOR_B_333333;
    static const cocos2d::Color3B COLOR_B_222222;
    static const cocos2d::Color3B COLOR_B_e94253;
    static const cocos2d::Color3B COLOR_B_37c267;
    static const cocos2d::Color3B COLOR_B_8b6bff;
    static const cocos2d::Color3B COLOR_B_8768f8;
    static const cocos2d::Color3B COLOR_B_26cec1;
    static const cocos2d::Color3B COLOR_B_eeeeee;
    static const cocos2d::Color3B COLOR_B_cccccc;
    static const cocos2d::Color3B COLOR_B_f3f3f3;
    
    static const cocos2d::Color4B COLOR_4B_WHITE;
    static const cocos2d::Color4B COLOR_4B_BLACK;
    static const cocos2d::Color4B COLOR_4B_EEEFF1;
    static const cocos2d::Color4B COLOR_4B_ADAFB3;
    static const cocos2d::Color4B COLOR_4B_DBDCDF;
    static const cocos2d::Color4B COLOR_4B_666666;
    static const cocos2d::Color4B COLOR_4B_999999;
    static const cocos2d::Color4B COLOR_4B_dddddd;
    static const cocos2d::Color4B COLOR_4B_333333;
    static const cocos2d::Color4B COLOR_4B_222222;
    static const cocos2d::Color4B COLOR_4B_e94253;
    static const cocos2d::Color4B COLOR_4B_37c267;
    static const cocos2d::Color4B COLOR_4B_8b6bff;
    static const cocos2d::Color4B COLOR_4B_8768f8;
    static const cocos2d::Color4B COLOR_4B_26cec1;
    static const cocos2d::Color4B COLOR_4B_eeeeee;
    static const cocos2d::Color4B COLOR_4B_cccccc;
    static const cocos2d::Color4B COLOR_4B_f3f3f3;
};


#endif /* SMFontColor_h */


/*
 class FTFontClass
 {
 public:
 static constexpr const char* FTFontMontSerratRegular = "fonts/Montserrat-Regular.ttf";
 static constexpr const char* FTFontMontSerratBold = "fonts/Montserrat-Bold.ttf";
 static constexpr const char* FTFontHelveticaLight = "fonts/Helvetica-light.ttf";
 static constexpr const char* FTFontHelveticaRegular = "fonts/Helvetica.ttf";
 static constexpr const char* FTFontHelveticaBold = "fonts/Helvetica-bold.otf";
 static constexpr const char* FTFontOvoRegular = "fonts/Ovo-Regular.otf";
 static constexpr const char* FTFontEncodeSansRegular = "fonts/ENCODESANSNORMAL-400-REGULAR.TTF";
 static constexpr const char* FTFontEncodeSansMedium = "fonts/ENCODESANSNORMAL-500-MEDIUM.TTF";
 
 
 #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
 static constexpr const char* FTFontSystemHelveticaRegular = "sans-serif";
 static constexpr const char* FTFontSystemHelveticaNeueMedium = "sans-serif-medium";
 static constexpr const char* FTFontSystemHelveticaBold = "sans-serif-condensed";
 static constexpr const char* FTFontSystemHelveticalNeueLight = "sans-serif-light";
 #else
 //    #if USE_HELVETICA_BASIC
 //        static constexpr const char* FTFontSystemHelveticaRegular = "Helvetica";
 //        static constexpr const char* FTFontSystemHelveticaNeueMedium = "HelveticaNeue-Medium";
 //        static constexpr const char* FTFontSystemHelveticaBold = "Helvetica-Bold";
 //        static constexpr const char* FTFontSystemHelveticalNeueLight = "HelveticaNeue-Light";
 //    #else
 
 static constexpr const char* FTFontSystemHelveticaRegular = "HelveticaNeue";
 static constexpr const char* FTFontSystemHelveticaNeueMedium = "HelveticaNeue-Medium";
 static constexpr const char* FTFontSystemHelveticaBold = "HelveticaNeue-Bold";
 static constexpr const char* FTFontSystemHelveticalNeueLight = "HelveticaNeue-Light";
 //    #endif
 #endif
 static constexpr const char* FTFontSystemCourierRegular = "Courier";
 static constexpr const char* FTFontSystemCourierBold = "Courier-Bold";
 };
 
 class FTColorClass
 {
 public:
 static const cocos2d::Color4F KEY_COLOR4F;
 static const cocos2d::Color4B KEY_COLOR4B;
 static const cocos2d::Color3B KEY_COLOR3B;
 static const cocos2d::Color4F KEY_COLOR_DIM4F;
 static const cocos2d::Color4F KEY_BLACK4F;
 static const cocos2d::Color4B KEY_BLACK4B;
 static const cocos2d::Color3B KEY_BLACK3B;
 static const cocos2d::Color4F KEY_BLACK_DIM4F;
 
 static const cocos2d::Color4F KEY_GRAY4F;
 static const cocos2d::Color4B KEY_GRAY4B;
 static const cocos2d::Color3B KEY_GRAY3B;
 static const cocos2d::Color4F KEY_GRAY_DIM4F;
 
 static const cocos2d::Color4F KEY_LIGHT_GRAY4F;
 static const cocos2d::Color4B KEY_LIGHT_GRAY4B;
 static const cocos2d::Color3B KEY_LIGHT_GRAY3B;
 static const cocos2d::Color4F KEY_LIGHT_GRAY_DIM4F;
 
 static const cocos2d::Color4F MINT4F;
 
 static const cocos2d::Color4F PRICE_RED4F;
 static const cocos2d::Color4B PRICE_RED4B;
 
 static const cocos2d::Color4F EDIT_BG4F;
 
 static const cocos2d::Color4F TRANSPARENT4F;
 
 static const cocos2d::Color4F KEY_GREEN_F;    //0x00b148
 static const cocos2d::Color4F KEY_ORANGE_F;   //0xe79f2b
 static const cocos2d::Color4F KEY_BLACK_F;    //0x222222
 static const cocos2d::Color4F KEY_DARK_GRAY_F;    //0x666666
 static const cocos2d::Color4F KEY_LIGHT_GRAY_F;   //0x999999
 static const cocos2d::Color4F KEY_DARK_WHITE_F;   //0xdddddd
 
 static const cocos2d::Color4B KEY_GREEN_B;    //0x00b148
 static const cocos2d::Color4B KEY_ORANGE_B;   //0xe79f2b
 static const cocos2d::Color4B KEY_BLACK_B;    //0x222222
 static const cocos2d::Color4B KEY_DARK_GRAY_B;    //0x666666
 static const cocos2d::Color4B KEY_LIGHT_GRAY_B;   //0x999999
 static const cocos2d::Color4B KEY_DARK_WHITE_B;   //0xdddddd
 
 static const cocos2d::Color4F COLOR_F_0x37c267;
 static const cocos2d::Color4F COLOR_F_0x00b148;
 static const cocos2d::Color4F COLOR_F_0x222222;
 static const cocos2d::Color4F COLOR_F_0x999999;
 static const cocos2d::Color4F COLOR_F_0xbbbbbb;
 
 static const cocos2d::Color4B COLOR_B_0x37c267;
 static const cocos2d::Color4B COLOR_B_0x00b148;
 static const cocos2d::Color4B COLOR_B_0x222222;
 static const cocos2d::Color4B COLOR_B_0x999999;
 static const cocos2d::Color4B COLOR_B_0xbbbbbb;
 
 
 static const cocos2d::Color4F COLOR_F_0x333333;
 static const cocos2d::Color4F COLOR_F_0x666666;
 static const cocos2d::Color4F COLOR_F_0xe94253;
 
 static const cocos2d::Color4B COLOR_B_0x333333;
 static const cocos2d::Color4B COLOR_B_0x666666;
 static const cocos2d::Color4B COLOR_B_0xe94253;
 };
 
*/
