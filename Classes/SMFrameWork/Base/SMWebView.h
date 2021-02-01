//
//  SMWebView.h
//  IMKSP
//
//  Created by SteveMac on 2017. 11. 3..
//

#ifndef SMWebView_h
#define SMWebView_h
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "SMView.h"
#include <ui/UIWebView.h>
#include <cocos2d.h>

typedef std::function<void(cocos2d::experimental::ui::WebView *sender, const std::string &url)> WebCallback;
typedef std::function<void(cocos2d::experimental::ui::WebView *sender, const std::string &scheme, const std::string &body)> WebJSIFCallback;
typedef std::function<void(cocos2d::experimental::ui::WebView *sender, const int keyCode)> KeyCallback;


class SMWebView : public SMView
{
public:
    static SMWebView * create(float x=0, float y=0, float width=0, float height=0, float anchorPointX=0, float anchorPointY=0);
    
    void loadURL(const std::string& url);
    void loadHTML(const std::string& html);
    void loadPostURL(const std::string& url, const std::string& postDataString);
    
    void enableWebViewContainerScroll(bool enable);
    
    void setOnShouldStartLoading(std::function<bool(cocos2d::experimental::ui::WebView * sender, const std::string &url)> callback);
    void setOnDidFailLoading(WebCallback callback);
    void setOnDidFinishLoading(WebCallback callback);
    void setOnJSCallback(WebCallback callback);
    void setOnJSIFCallback(WebJSIFCallback callback);
    void setOnKeyDown(KeyCallback callback);
    void setOnKeyUp(KeyCallback callback);
    
    void evaluateJS(const std::string &js);
    
    void setScalesPageToFit(bool bScalePageToFit);
    
    void captureWebPage(std::function<void(cocos2d::Image*)> capturedComplete);
    
    void setJavascriptInterfaceScheme(const std::string &scheme);
    
    virtual void setVisible(bool visible) override;
    
    void stopLoading();
    
    void reload();
    bool canGoBack();
    bool canGoForward();
    void goBack();
    void goForward();
    
    std::string getCurrentLoadUrl();
    
    void clear();
    
    void showProgress();
    void hideProgress();
    
    cocos2d::experimental::ui::WebView* getImplWebView();
    
public:
    virtual void setContentSize(const cocos2d::Size& contentSize) override;
    
protected:
    SMWebView();
    virtual ~SMWebView();
    
    virtual bool init() override;
    
private:
    bool onShouldStartLoading(cocos2d::experimental::ui::WebView* sender, const std::string& url);
    void onDidFailLoading(cocos2d::experimental::ui::WebView* sender, const std::string& url);
    void onDidFinishLoading(cocos2d::experimental::ui::WebView* sender, const std::string& url);
    void onJSCallback(cocos2d::experimental::ui::WebView* sender, const std::string& url);
    void onJSIFCallback(cocos2d::experimental::ui::WebView* sender, const std::string& scheme, const std::string& body);
    
private:
    cocos2d::experimental::ui::WebView* _webView;
    std::function<bool(cocos2d::experimental::ui::WebView* sender, const std::string& url)> _startLoadingCallback;
    WebCallback _failLoadingCallback;
    WebCallback _finishLoadingCallback;
    WebCallback _jsCallback;
    WebJSIFCallback _jsIFCallback;
    KeyCallback _keyDownCallback;
    KeyCallback _keyUpCallback;
    
    std::string _currentUrl;
};
#endif

#endif /* SMWebView_h */
