//
//  SMFontColor.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 14..
//
//

#include "SMFontColor.h"
#include "../Util/ViewUtil.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
const char* SMFontConst::SystemFontRegular = "Arial";
const char* SMFontConst::SystemFontBold = "Arial-medium";
const char* SMFontConst::SystemFontLight = "Arial-light";
#elif CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_MAC
const char* SMFontConst::SystemFontRegular = "Helvetica";
const char* SMFontConst::SystemFontBold = "Helvetica-Bold";
const char* SMFontConst::SystemFontLight = "Helvetica-Light"; // do not use
#else
const char* SMFontConst::SystemFontRegular = "sans-serif";
const char* SMFontConst::SystemFontBold = "sans-serif-medium";
const char* SMFontConst::SystemFontLight = "sans-serif-light";
#endif

const char* SMFontConst::MontSerratRegular   = "fonts/Montserrat-Regular.ttf";
const char* SMFontConst::MontSerratBold      = "fonts/Montserrat-Bold.ttf";

const char* SMFontConst::NotoSansLight = "fonts/NotoSansKR-Light.otf";
const char* SMFontConst::NotoSansRegular = "fonts/NotoSansKR-Regular.otf";
const char* SMFontConst::NotoSansMedium = "fonts/NotoSansKR-Medium.otf";
const char* SMFontConst::NotoSansBold = "fonts/NotoSansKR-Bold.otf";


const char* SMFontConst::SFProDisplayThin = "fonts/SFProDisplay-Thin.ttf";
const char* SMFontConst::SFProDisplayLight = "fonts/SFProDisplay-Light.ttf";
const char* SMFontConst::SFProDisplayMedium = "fonts/SFProDisplay-Medium.ttf";
const char* SMFontConst::SFProDisplayBold = "fonts/SFProDisplay-Bold.ttf";



// RGB, A
const cocos2d::Color4F SMColorConst::COLOR_F_WHITE = MAKE_COLOR4F(0xffffff, 1);;
const cocos2d::Color4F SMColorConst::COLOR_F_BLACK = MAKE_COLOR4F(0x000000, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_EEEFF1 = MAKE_COLOR4F(0xeeeff1, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_ADAFB3 = MAKE_COLOR4F(0xadafb3, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_DBDCDF = MAKE_COLOR4F(0xdbdcdf, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_666666 = MAKE_COLOR4F(0x666666, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_999999 = MAKE_COLOR4F(0x999999, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_dddddd = MAKE_COLOR4F(0xdddddd, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_333333 = MAKE_COLOR4F(0x333333, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_222222 = MAKE_COLOR4F(0x222222, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_e94253 = MAKE_COLOR4F(0xe94253, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_37c267 = MAKE_COLOR4F(0x37c267, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_8b6bff = MAKE_COLOR4F(0x8b6bff, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_8768f8 = MAKE_COLOR4F(0x8768f8, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_26cec1 = MAKE_COLOR4F(0x26cec1, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_eeeeee = MAKE_COLOR4F(0xeeeeee, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_cccccc = MAKE_COLOR4F(0xcccccc, 1);
const cocos2d::Color4F SMColorConst::COLOR_F_f3f3f3 = MAKE_COLOR4F(0xf3f3f3, 1);

// RGB
const cocos2d::Color3B SMColorConst::COLOR_B_WHITE = MAKE_COLOR3B(0xffffff);
const cocos2d::Color3B SMColorConst::COLOR_B_BLACK = MAKE_COLOR3B(0x000000);
const cocos2d::Color3B SMColorConst::COLOR_B_EEEFF1 = MAKE_COLOR3B(0xeeeff1);
const cocos2d::Color3B SMColorConst::COLOR_B_ADAFB3 = MAKE_COLOR3B(0xadafb3);
const cocos2d::Color3B SMColorConst::COLOR_B_DBDCDF = MAKE_COLOR3B(0xdbdcdf);
const cocos2d::Color3B SMColorConst::COLOR_B_666666 = MAKE_COLOR3B(0x666666);
const cocos2d::Color3B SMColorConst::COLOR_B_999999 = MAKE_COLOR3B(0x999999);
const cocos2d::Color3B SMColorConst::COLOR_B_dddddd = MAKE_COLOR3B(0xdddddd);
const cocos2d::Color3B SMColorConst::COLOR_B_333333 = MAKE_COLOR3B(0x333333);
const cocos2d::Color3B SMColorConst::COLOR_B_222222 = MAKE_COLOR3B(0x222222);
const cocos2d::Color3B SMColorConst::COLOR_B_e94253 = MAKE_COLOR3B(0xe94253);
const cocos2d::Color3B SMColorConst::COLOR_B_37c267 = MAKE_COLOR3B(0x37c267);
const cocos2d::Color3B SMColorConst::COLOR_B_8b6bff = MAKE_COLOR3B(0x8b6bff);
const cocos2d::Color3B SMColorConst::COLOR_B_8768f8 = MAKE_COLOR3B(0x8768f8);
const cocos2d::Color3B SMColorConst::COLOR_B_26cec1 = MAKE_COLOR3B(0x26cec1);
const cocos2d::Color3B SMColorConst::COLOR_B_eeeeee = MAKE_COLOR3B(0xeeeeee);
const cocos2d::Color3B SMColorConst::COLOR_B_cccccc = MAKE_COLOR3B(0xcccccc);
const cocos2d::Color3B SMColorConst::COLOR_B_f3f3f3 = MAKE_COLOR3B(0xf3f3f3);

// ARGB
const cocos2d::Color4B SMColorConst::COLOR_4B_WHITE = MAKE_COLOR4B(0xffffffff);
const cocos2d::Color4B SMColorConst::COLOR_4B_BLACK = MAKE_COLOR4B(0xff000000);
const cocos2d::Color4B SMColorConst::COLOR_4B_EEEFF1 = MAKE_COLOR4B(0xffeeeff1);
const cocos2d::Color4B SMColorConst::COLOR_4B_ADAFB3 = MAKE_COLOR4B(0xffadafb3);
const cocos2d::Color4B SMColorConst::COLOR_4B_DBDCDF = MAKE_COLOR4B(0xffdbdcdf);
const cocos2d::Color4B SMColorConst::COLOR_4B_666666 = MAKE_COLOR4B(0xff666666);
const cocos2d::Color4B SMColorConst::COLOR_4B_999999 = MAKE_COLOR4B(0xff999999);
const cocos2d::Color4B SMColorConst::COLOR_4B_dddddd = MAKE_COLOR4B(0xffdddddd);
const cocos2d::Color4B SMColorConst::COLOR_4B_333333 = MAKE_COLOR4B(0xff333333);
const cocos2d::Color4B SMColorConst::COLOR_4B_222222 = MAKE_COLOR4B(0xff222222);
const cocos2d::Color4B SMColorConst::COLOR_4B_e94253 = MAKE_COLOR4B(0xffe94253);
const cocos2d::Color4B SMColorConst::COLOR_4B_37c267 = MAKE_COLOR4B(0xff37c267);
const cocos2d::Color4B SMColorConst::COLOR_4B_8b6bff = MAKE_COLOR4B(0xff8b6bff);
const cocos2d::Color4B SMColorConst::COLOR_4B_8768f8 = MAKE_COLOR4B(0xff8768f8);
const cocos2d::Color4B SMColorConst::COLOR_4B_26cec1 = MAKE_COLOR4B(0xff26cec1);
const cocos2d::Color4B SMColorConst::COLOR_4B_eeeeee = MAKE_COLOR4B(0xffeeeeee);
const cocos2d::Color4B SMColorConst::COLOR_4B_cccccc = MAKE_COLOR4B(0xffcccccc);
const cocos2d::Color4B SMColorConst::COLOR_4B_f3f3f3 = MAKE_COLOR4B(0xfff3f3f3);
