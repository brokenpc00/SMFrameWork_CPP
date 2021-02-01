//
//  SMComm.cpp
//  iShop
//
//  Created by KimSteve on 2016. 12. 6..
//
// Network Wrapping class

#include "SMComm.h"
#include <cocos2d.h>


// for test... 지금은 fituin server로 접속해보자..

#define FOR_DEV 0
#define FOR_US 1

#if FOR_DEV
    // 개발서버
    #define SERVER_URL "http://openapidev.fituin."
    #if FOR_US
        // 미국서버
        #define NATIONAL_CODE   "us/"
    #else
        // 한국서버
        #define NATIONAL_CODE   "kr/"
    #endif

#else
    // 실서버
    #define SERVER_URL "https://openapi.fituin."
    #if FOR_US
        // 미국서버
        #define NATIONAL_CODE   "us/"
    #else
        // 한국서버
        #define NATIONAL_CODE   "kr/"
    #endif
#endif


// fituin에서 서버구성이 일케 되어 있음... 프로젝트마다 다시 설정
#define OAUTH_PATH                                          "oauth2/"
#define AUTH_PATH                                           "auth/"


// 일단 이것만 테스트 해보자... Init 정보를 불러오는 거임...
#define API_PATH_INIT_INFO                                          "initInfo/list"


// oauth2
#define API_PATH_SIGN_UP                                "signUp"    // post
#define API_PATH_SIGN_IN                                "login"         // post
#define API_PATH_TOKEN                                  "token"         // post






// task 관리용 Identification String Table
const std::string SMComm::kCommAPIInitInfo = "INIT INFO";








// url encode용 static 함수
static std::string url_encode(const std::string &s)
{
    const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
    
    std::string escaped="";
    for(size_t i=0; i<s.length(); i++)
    {
        if (unreserved.find_first_of(s[i]) != std::string::npos)
        {
            escaped.push_back(s[i]);
        }
        else
        {
            escaped.append("%");
            char buf[3];
            sprintf(buf, "%.2X", (unsigned char)s[i]);
            escaped.append(buf);
        }
    }
    return escaped;
}




static SMComm * _instance = nullptr;

SMComm * SMComm::getInstance()
{
    if (_instance==NULL) {
        _instance = new SMComm();
    }
    
    return _instance;
}

SMComm::SMComm() :
_initNode(nullptr)
{
    _callbackStack.clear();
    _callbackSharedStack.clear();
}

SMComm::~SMComm()
{
    
}

// 통신 완료 callback
// 받은 통신 task를 찾아 호출자에게 전달한 후 제거한다.
void SMComm::onHttpRequestComplete(cocos2d::network::HttpClient *sender, cocos2d::network::HttpResponse *response)
{
    if (response->isSucceed()) {
        
        // 현재 callback task stack을 복사
        _mutex.lock();
        std::vector<std::weak_ptr<ApiTask>> tmpStack;
        tmpStack.insert(tmpStack.end(), _callbackStack.begin(), _callbackStack.end());
        _mutex.unlock();
        
        // delete용 callback task stack
        std::vector<std::weak_ptr<ApiTask>> delStack;
        
        std::string currentUrl = response->getHttpRequest()->getUrl();
        
        for (auto iter=tmpStack.begin(); iter!=tmpStack.end();) {
            auto sp = iter->lock();
            std::string compareUrl = sp->_request->getUrl();
            if (iter->expired()) {
                // 만기된 거면 del stack에 넣는다.
                delStack.push_back(*iter);
            } else if (currentUrl==compareUrl) {
                // 현재 URL이고 살아 있다면 호출자한테 넘겨줌.
                if (sp->isAlived()) {
                    if (sp->_callback) {
                        sp->_callback(sender, response);
                    }
                }
                // 사용했으니 필요 없으므로 del stack에 넣는다.
                delStack.push_back(*iter);
            } else if (!sp->isAlived()) {
                // 살아 있지 않다면 del stack에 넣는다.
                delStack.push_back(*iter);
            }
            ++iter;
        }
        
        
        
        // del stack에 있는 task들을 실제 callback stak에서 삭제한다.
        _mutex.lock();
        if (delStack.size()>0) {
            
            for (int i=0; i<delStack.size(); i++) {
                auto sp = delStack[i].lock();
                for (auto iter=_callbackStack.begin(); iter!=_callbackStack.end();) {
                    auto compareSP = iter->lock();
                    if (sp==compareSP) {
                        iter = _callbackStack.erase(iter);
                        int nPos = -1;
                        for (int j=0; j<_callbackSharedStack.size(); j++) {
                            if (sp==_callbackSharedStack[j]) {
                                nPos = j;
                                break;
                            }
                        }
                        if (nPos>-1) {
                            auto iter2 = _callbackSharedStack.begin();
                            std::advance(iter2, nPos);
                            _callbackSharedStack.erase(iter2);
                        }
                    } else {
                        ++iter;
                    }
                }
            }
        }
        _mutex.unlock();
        // del stack을 비운다.
        delStack.clear();
    }
}

void SMComm::getHtmlContents(ApiListener * listener, std::string url, const CommCallback &callback)
{
    cocos2d::network::HttpRequest * request = new cocos2d::network::HttpRequest();
    
    request->setUrl(url.c_str());
    request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
    request->setResponseCallback(CC_CALLBACK_2(SMComm::onHttpRequestComplete, this));
    request->setTag("GET HTML");
    cocos2d::network::HttpClient::getInstance()->send(request);
    request->release();
}

void SMComm::addCallbackStack(std::shared_ptr<ApiTask> callback)
{
    _mutex.lock();
    
    _callbackSharedStack.emplace_back(callback);
    std::weak_ptr<ApiTask> wp(callback);
    _callbackStack.emplace_back(wp);
    _mutex.unlock();
}

void SMComm::removeCallbackStack(std::shared_ptr<ApiTask> callback)
{
    _mutex.lock();
    for (auto iter=_callbackStack.begin(); iter!=_callbackStack.end();) {
        auto sp = iter->lock();
        if (sp) {
            if (sp==callback) {
                iter = _callbackStack.erase(iter);
            } else {
                ++iter;
            }
        } else if (sp==nullptr) {
            iter = _callbackStack.erase(iter);
        } else {
            ++iter;
        }
    }
    _mutex.unlock();
}


// url type으로 실제 호출될 URL 전체 경로를 반환한다.
std::string SMComm::getBaseURL(kFTCommURLType type)
{
    std::string url = cocos2d::StringUtils::format("%s%s%s%s", SERVER_URL, NATIONAL_CODE, OAUTH_PATH, API_PATH_SIGN_UP);
    std::string server = SERVER_URL;
    std::string national = NATIONAL_CODE;
    std::string oauth;
    std::string apiPath;
    std::string retURL;
    switch (type) {
        case kFTCommURLTypeInitInfo:
        {
            oauth = OAUTH_PATH;
            apiPath = API_PATH_INIT_INFO;
        }
            break;
    }
    
    retURL = cocos2d::StringUtils::format("%s%s%s%s", server.c_str(), national.c_str(), oauth.c_str(), apiPath.c_str());
    return retURL;
}

// 파라미터 생성용 함수
std::string SMComm::makeCommParams(std::unordered_map<std::string, std::string> params)
{
    if (params.size()==0) {
        return "";
    }
    
    // parameter sort
    std::list<std::string> paramKeys;
    for (auto kv : params) {
        paramKeys.push_back(kv.first);
    }
    
    paramKeys.sort();
    char paramString[2048];
    memset(paramString, 0, 2048);
    bool bFirst = true;
    for (auto itr : paramKeys) {
        std::string key(itr.data());
        std::string value(params[key]);
        if (!bFirst) {
            strcat(paramString, "&");
        } else {
            bFirst = false;
        }
        strcat(paramString, url_encode(key).data());
        strcat(paramString, "=");
        strcat(paramString, url_encode(value).data());
    }
    
    return paramString;
}

// 로그인 정보(토큰)... 나중에 구현.
std::string SMComm::getAuthString()
{
    return "";
}


std::vector<std::string> SMComm::getBaseHeader()
{
    const std::string base64String = "Basic YmFhMWQ2OWQyNDUyYmI1YzE1ZTNjY2M2ODI2MTk1OGI6NDNlMzgyZDIyMjA5NjlkMWY0ZWE5NDMwODY4YzMxOTI=";
    const std::string authHeaderString = "Authorization:" + base64String;
    
    std::vector<std::string> headers;
    headers.clear();
    headers.push_back(authHeaderString);
    headers.push_back("Content-Type: application/x-www-form-urlencoded; charset=utf-8");
    
    return headers;
}

std::vector<std::string> SMComm::getBaseHeaderWithToken(std::string authString)
{
    std::string authHeaderString = cocos2d::StringUtils::format("Authorization:%s", authString.c_str());
    std::vector<std::string> headers;
    headers.clear();
    headers.push_back(authHeaderString);
    
    return headers;
}

void SMComm::sendAPIWithTokenImmediate(ApiListener * listener, kFTCommURLType type, std::unordered_map<std::string, std::string> params, std::string authString, std::string Tag, const cocos2d::network::precedenceCallback &preCallback, const cocos2d::network::flushDataCallback& flushCallback, const CommCallback& callback) {
    sendAPIWithToken(listener, type, params, authString, Tag, preCallback, flushCallback, callback);
}

void SMComm::sendAPIWithToken(ApiListener * listener, kFTCommURLType type, std::unordered_map<std::string, std::string> params, std::string authString, std::string Tag, const cocos2d::network::precedenceCallback &preCallback, const cocos2d::network::flushDataCallback& flushCallback, const CommCallback& callback, bool immediate)
{
    params["locale"] = "KO";
    params["nationCode"] = "kr";
    
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    params["os"] = "android";
#else
    params["os"] = "ios";
#endif

    std::string paramString = makeCommParams(params);
    
    std::string url = getBaseURL(type);
    
    
    cocos2d::network::HttpRequest * request = new cocos2d::network::HttpRequest();
    
    // GET or POST 구분
    if (0) {
        // POST
        request->setRequestData(paramString.c_str(), paramString.length());
        request->setRequestType(cocos2d::network::HttpRequest::Type::POST);
        request->setUrl(url.c_str());
    } else {
        // GET
        request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
        url = cocos2d::StringUtils::format("%s?%s", url.c_str(), paramString.c_str());
        request->setUrl(url.c_str());        
    }
    
    request->setPrecedenceCallback(preCallback);
    request->setFlushCallback(flushCallback);
    request->setResponseCallback(CC_CALLBACK_2(SMComm::onHttpRequestComplete, this));
    request->setHeaders(getBaseHeaderWithToken(authString));
    request->setTag(Tag.c_str());
    
    if (listener) {
        listener->nCallType = type;
        std::shared_ptr<ApiTask> sp = std::make_shared<ApiTask>();
        sp->_listener = listener;
        sp->_request = request;
        sp->_alive = listener->_alive;
        sp->_callback = callback;
        addCallbackStack(sp);
    }
    
    if (immediate) {
        cocos2d::network::HttpClient::getInstance()->sendImmediate(request);
    } else {
        cocos2d::network::HttpClient::getInstance()->send(request);
    }
    
    request->release();
}


// 비 인증용 api
void SMComm::sendAPIImmediate(ApiListener * listener, kFTCommURLType type, std::unordered_map<std::string, std::string> params, std::string Tag, const cocos2d::network::precedenceCallback &preCallback, const cocos2d::network::flushDataCallback& flushCallback, const CommCallback& callback) {
    sendAPI(listener, type, params, Tag, preCallback, flushCallback, callback, true);
}

void SMComm::sendAPI(ApiListener * listener, kFTCommURLType type, std::unordered_map<std::string, std::string> params, std::string Tag, const cocos2d::network::precedenceCallback &preCallback, const cocos2d::network::flushDataCallback& flushCallback, const CommCallback& callback, bool immediate)
{
    params["locale"] = "KO";
    params["nationCode"] = "kr";
    
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    params["os"] = "android";
#else
    params["os"] = "ios";
#endif

    std::string paramString = makeCommParams(params);
    
    std::string url = getBaseURL(type);
    
    cocos2d::network::HttpRequest * request = new cocos2d::network::HttpRequest();
    
    if (0) {
        // POST
        request->setRequestData(paramString.c_str(), paramString.length());
        request->setRequestType(cocos2d::network::HttpRequest::Type::POST);
        request->setUrl(url.c_str());
    } else {
        // GET
        request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
        url = cocos2d::StringUtils::format("%s?%s", url.c_str(), paramString.c_str());
        request->setUrl(url.c_str());
    }
    
    // 전처리기... 통신 완료 후에 쓰레드 상태에서 파싱등을 하기 위한 전처리 함수
    request->setPrecedenceCallback(preCallback);
    
    // 후처러기... 통신 완료 후 호출자에게 통신 완료 정보까지 넘겨준 후 후처리 함수
    request->setFlushCallback(flushCallback);
    
    // 통신 완료 callback
    request->setResponseCallback(CC_CALLBACK_2(SMComm::onHttpRequestComplete, this));
    request->setHeaders(getBaseHeader());
    request->setTag(Tag.c_str());

    if (listener) {
        listener->nCallType = type;
        std::shared_ptr<ApiTask> sp = std::make_shared<ApiTask>();
        sp->_listener = listener;
        sp->_request = request;
        sp->_alive = listener->_alive;
        sp->_callback = callback;
        addCallbackStack(sp);
    }
    
    if (immediate) {
        cocos2d::network::HttpClient::getInstance()->sendImmediate(request);
    } else {
        cocos2d::network::HttpClient::getInstance()->send(request);
    }
    request->release();
}





// 테스트 호출용....지금은 이것밖에 호출할게 없음...
// 나중에 프로젝트에서 추가하면 됨.
void SMComm::initInfo(ApiListener * listener, std::string appVerString, std::string cateVerString, const CommCallback& callback)
{
    _initNode = cocos2d::Node::create();
    _initNode->retain();
    cocos2d::network::HttpClient::getInstance()->cancelSend(SMComm::kCommAPIInitInfo.c_str());
    std::unordered_map<std::string, std::string> params;
    params["appVer"] = appVerString;
    params["cateVersion"] = cateVerString;
    
    // 전처리기
    cocos2d::network::precedenceCallback preCallback = [&](cocos2d::network::HttpResponse * response)->void*{
        if (response->isSucceed()) {
        }
        CCLOG("[[[[[ Init info precendence callback");
        return NULL;
    };
    
    // 후처리기
    cocos2d::network::flushDataCallback flushCallback = [&](cocos2d::network::HttpResponse * response){
        if (response->getParseData()) {
        }
        CCLOG("[[[[[ Init info flush callback");
    };
    sendAPIImmediate(listener, kFTCommURLTypeInitInfo, params, SMComm::kCommAPIInitInfo, preCallback, flushCallback, callback);
}

