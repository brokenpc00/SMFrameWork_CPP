//
//  SMWebView.cpp
//  IMKSP
//
//  Created by SteveMac on 2017. 11. 3..
//

#include "SMWebView.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

SMWebView* SMWebView::create(float x, float y, float width, float height, float anchorPointX, float anchorPointY)
{
    SMWebView * view = new (std::nothrow)SMWebView();
    if (view!=nullptr) {
        view->setContentSize(cocos2d::Size(width, height));
        view->setAnchorPoint(cocos2d::Vec2(anchorPointX, anchorPointY));
        view->setPosition(cocos2d::Vec2(x, y));
        if (view->init()) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    
    return view;
}

SMWebView::SMWebView() : _webView(nullptr)
, _startLoadingCallback(nullptr)
, _failLoadingCallback(nullptr)
, _finishLoadingCallback(nullptr)
, _jsCallback(nullptr)
, _currentUrl("")
{
    
}

SMWebView::~SMWebView()
{
    if (_webView) {
        _startLoadingCallback = nullptr;
        _failLoadingCallback = nullptr;
        _finishLoadingCallback = nullptr;
        _jsCallback = nullptr;
        _jsIFCallback = nullptr;
        _webView->setOnShouldStartLoading(nullptr);
        _webView->setOnDidFailLoading(nullptr);
        _webView->setOnDidFinishLoading(nullptr);
        _webView->setOnJSCallback(nullptr);
        _webView->setOnJSIFCallback(nullptr);
        _webView->stopLoading();
        _webView->hideProgress();
        _webView->hideKeyboard();
    }
}

bool SMWebView::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    setBackgroundColor4F(cocos2d::Color4F::WHITE);
    _webView = cocos2d::experimental::ui::WebView::create();
    _webView->setAnchorPoint(cocos2d::Vec2::ZERO);
    addChild(_webView);
    _webView->setOnShouldStartLoading(CC_CALLBACK_2(SMWebView::onShouldStartLoading, this));
    _webView->setOnDidFinishLoading(CC_CALLBACK_2(SMWebView::onDidFinishLoading, this));
    _webView->setOnDidFailLoading(CC_CALLBACK_2(SMWebView::onDidFailLoading, this));
    _webView->setOnJSCallback(CC_CALLBACK_2(SMWebView::onJSCallback, this));
    _webView->setOnJSIFCallback(CC_CALLBACK_3(SMWebView::onJSIFCallback, this));
    //_inputEditBox->setSwallowTouches(false);
    _webView->setSwallowTouches(false);
    return true;
}

void SMWebView::setContentSize(const cocos2d::Size &contentSize)
{
    SMView::setContentSize(contentSize);
    
    if (_webView) {
        _webView->setContentSize(contentSize);
    }
}

bool SMWebView::onShouldStartLoading(cocos2d::experimental::ui::WebView *sender, const std::string &url)
{
//    CCLOG("[[[[[WEBVEW start loading : %s", url.c_str());
    if (url=="about:blank") {
        return false;
    }
    if (_startLoadingCallback!=nullptr) {
        _startLoadingCallback(sender, url);
    }
    return true;
}

void SMWebView::onDidFailLoading(cocos2d::experimental::ui::WebView *sender, const std::string &url)
{
    if (_failLoadingCallback) {
        _failLoadingCallback(sender, url);
    }
}

void SMWebView::onDidFinishLoading(cocos2d::experimental::ui::WebView *sender, const std::string &url)
{
    if (_finishLoadingCallback) {
        _finishLoadingCallback(sender, url);
    }
}

void SMWebView::onJSCallback(cocos2d::experimental::ui::WebView *sender, const std::string &url)
{
    if (_jsCallback) {
        _jsCallback(sender, url);
    }
}

void SMWebView::onJSIFCallback(cocos2d::experimental::ui::WebView* sender, const std::string& scheme, const std::string& body)
{
    if (_jsIFCallback) {
        _jsIFCallback(sender, scheme, body);
    }
}

std::string SMWebView::getCurrentLoadUrl()
{
    return _currentUrl;
}

void SMWebView::loadURL(const std::string &url)
{
    _currentUrl = url;
    _webView->loadURL(url);
}

void SMWebView::loadHTML(const std::string &html)
{
    _webView->loadHTMLString(html);
}

void SMWebView::enableWebViewContainerScroll(bool enable)
{
    _webView->enableWebViewContainerScroll(enable);
}

void SMWebView::setOnShouldStartLoading(std::function<bool(cocos2d::experimental::ui::WebView * sender, const std::string& url)> callback)
{
    _startLoadingCallback = callback;
}

void SMWebView::setOnDidFailLoading(WebCallback callback)
{
    _failLoadingCallback = callback;
}

void SMWebView::setOnDidFinishLoading(WebCallback callback)
{
    _finishLoadingCallback = callback;
}

void SMWebView::setOnJSCallback(WebCallback callback)
{
    _jsCallback = callback;
}

void SMWebView::setOnJSIFCallback(WebJSIFCallback callback)
{
    _jsIFCallback = callback;
}

void SMWebView::evaluateJS(const std::string &js)
{
    _webView->evaluateJS(js);
}

void SMWebView::setScalesPageToFit(bool bScalePageToFit)
{
    _webView->setScalesPageToFit(bScalePageToFit);
}

void SMWebView::captureWebPage(std::function<void(cocos2d::Image*)> capturedBlock)
{
    _webView->captureWebPage(capturedBlock);
}

void SMWebView::loadPostURL(const std::string& url, const std::string& postDataString)
{
    _webView->loadPostURL(url, postDataString);
}

void SMWebView::setJavascriptInterfaceScheme(const std::string &scheme)
{
    _webView->setJavascriptInterfaceScheme(scheme);
}

void SMWebView::setVisible(bool visible)
{
    cocos2d::Node::setVisible(visible);
    _webView->setVisible(visible);
}

void SMWebView::stopLoading()
{
    _webView->stopLoading();
}

void SMWebView::clear()
{
    _webView->stopLoading();
    _webView->clear();
}

void SMWebView::reload()
{
    _webView->reload();
}

bool SMWebView::canGoBack()
{
    return _webView->canGoBack();
}

bool SMWebView::canGoForward()
{
    return _webView->canGoForward();
}

void SMWebView::goBack()
{
    _webView->goBack();
}

void SMWebView::goForward()
{
    _webView->goForward();
}

void SMWebView::showProgress()
{
    _webView->showProgress();
}

void SMWebView::hideProgress()
{
    if (_webView) {
    _webView->hideProgress();
    }
}

cocos2d::experimental::ui::WebView* SMWebView::getImplWebView()
{
    return _webView;
}
#endif
