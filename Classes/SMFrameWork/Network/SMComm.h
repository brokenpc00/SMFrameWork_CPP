//
//  SMComm.h
//  iShop
//
//  Created by KimSteve on 2016. 12. 6..
//
// Network wrapping class

#ifndef SMComm_h
#define SMComm_h

#include <network/HttpClient.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

typedef enum {
    kCOMM_ERROR_CODE_SUCCESS = 0,
    kCOMM_ERROR_CODE_NO_PARAM = 1,
    
    kCOMM_ERROR_CODE_NOT_AUTHORIZE = 201,       // oauth error
    kCOMM_ERROR_CODE_FAIL_TO_AUTHORIZE = 203,       // password not matched
    kCOMM_ERROR_CODE_EXPIRE_TIME_EXCEEDED = 205,        // expire time exceeded
    kCOMM_ERROR_CODE_NOT_EXIST_RESULT = 302,        // not exist result
    kCOMM_ERROR_CODE_ACCESS_DENY = 403,         // access deny
    kCOMM_ERROR_CODE_NOT_FOUND = 414,       // not registered
    kCOMM_ERROR_CODE_SERVER_ERROR = 500,        // server system error
    kCOMM_ERROR_CODE_DUPLICATE = 601,       // already registered
    kCOMM_ERROR_CODE_UNKNOWN = 9999,
} kCOMM_ERROR_CODE;


typedef std::function<void(cocos2d::network::HttpClient *, cocos2d::network::HttpResponse *)> CommCallback;

class ApiListener
{
public:
    
    ApiListener() {
        _alive = std::make_shared<bool>(true);
    };
    virtual ~ApiListener(){};
    
public:
    int nCallType;
    std::shared_ptr<bool> _alive;
};

class ApiTask
{
public:
    ApiTask() {
        _callback = nullptr;
    }
    virtual ~ApiTask(){}
    
public:
    cocos2d::network::HttpRequest* _request;
    inline bool isAlived() {return !_alive.expired();}
    CommCallback _callback;
    ApiListener* _listener;
    std::weak_ptr<bool> _alive;
};

//cocos2d::network::precedenceCallback -> network이 끝나고 아직 쓰레드 상태에서 전처리기 (파싱등을 처리)
//cocos2d::network::flushDataCallback -> network이 끝나고 전처리기를 클리어...

class SMComm
{
public:
    static SMComm* getInstance();
    
    // comm method
    // for test... fituin init 정보를 불러본다.
    void initInfo(ApiListener * listener, std::string appVerString, std::string cateVerString, const CommCallback& callback);
    
    // html 긁어오기...
    void getHtmlContents(ApiListener * listener, std::string url, const CommCallback& callback);
    

    // scene에서 빠져나가거나 view가 죽을때.. 즉 호출자가 죽을때 자기가 호출했던 comm task를 삭제하기 위한 함수...
    // shared pointer를 사용하기 때문에 없어도 되지만.... 만약을 위해 방어코드로 삽입함.
    // running scene이 호출자가 아닐 수도 있기 때문에 각 scene에서 호출 해줘야함... (TransitionScene일 수도 있으므로...)
    void cancelFromNode(ApiListener * listener);
    
    
    
    
    
    
    // comm instance 관리용 string 시작
    static const std::string kCommAPIInitInfo;
    // comm instance 관리용 string 끝
    
    
    
    // comm URL 관리용 상수 시작
    typedef enum {
        kFTCommURLTypeInitInfo,
        // 계속 추가할 것...
        
    } kFTCommURLType;;
    // comm URL 관리용 상수 끝

    
protected:
    
    // comm task 관리용 property
    std::vector<std::weak_ptr<ApiTask>> _callbackStack;
    std::vector<std::shared_ptr<ApiTask>> _callbackSharedStack;
    
    
    
protected:
    // url type을 기준으로 전체 경로를 얻어오는 함수....인데.. 구현해야됨.
    std::string getBaseURL(kFTCommURLType type);
    
    // 실제 cocos2d가 통신이 완료된 시점의 listener
    void onHttpRequestComplete(cocos2d::network::HttpClient * sender, cocos2d::network::HttpResponse * response);
    
    // 내부에서 사용할 파라미터 조합 함수...  &와 =로 나열해주는 함수이다.
    std::string makeCommParams(std::unordered_map<std::string, std::string> params);

    // header 조합용 함수... OAuth 2.0 기준 함수이다... 프로젝트 마다 다르다.
    std::vector<std::string> getBaseHeader();
    // header 조합용 함수... OAuth 2.0 기준 함수이다... 프로젝트 마다 다르다... 이거는 로그인 상태에서 토큰값을 포함한다. 즉, 로그인 이후로는 이 함수가 호출된다.
    std::vector<std::string> getBaseHeaderWithToken(std::string authString);
    
    
    // comm task를 추가한다.
    void addCallbackStack(std::shared_ptr<ApiTask> callback);
    // comm task를 삭제한다.
    void removeCallbackStack(std::shared_ptr<ApiTask> callback);
    
    
    
    // 인증된(로그인된) 사용자의 경우 인증 문자열(토큰)을 불러오기 위한 함수.... 구현해야함.
    std::string getAuthString();
    
    // network api call용 함수
    // 기본 호출... 로그인 정보 없는 기본 API를 호출할 때 사용한다.
    void sendAPI(ApiListener * listener, kFTCommURLType type, std::unordered_map<std::string, std::string> params, std::string Tag, const cocos2d::network::precedenceCallback &preCallback, const cocos2d::network::flushDataCallback& flushCallback, const CommCallback& callback, bool immediate=false);
    // 즉시 호출... 로그인 정보 없는 기본 API를 호출할 때 사용한다.
    void sendAPIImmediate(ApiListener * listener, kFTCommURLType type, std::unordered_map<std::string, std::string> params, std::string Tag, const cocos2d::network::precedenceCallback &preCallback, const cocos2d::network::flushDataCallback& flushCallback, const CommCallback& callback);
    
    // 이거는 나중에 프로젝트 들어가면...
    // 인증했을 경우 token과 같이 기본호출... 이거는 로그인 상태에서 토큰값을 포함한다. 즉, 로그인 이후로는 이 함수가 호출된다.
    void sendAPIWithToken(ApiListener * listener, kFTCommURLType type, std::unordered_map<std::string, std::string> params, std::string authString, std::string Tag, const cocos2d::network::precedenceCallback &preCallback, const cocos2d::network::flushDataCallback& flushCallback, const CommCallback& callback, bool immediate=false);
    // 인증했을 경우 token과 같이 즉시호출... 이거는 로그인 상태에서 토큰값을 포함한다. 즉, 로그인 이후로는 이 함수가 호출된다.
    void sendAPIWithTokenImmediate(ApiListener * listener, kFTCommURLType type, std::unordered_map<std::string, std::string> params, std::string authString, std::string Tag, const cocos2d::network::precedenceCallback &preCallback, const cocos2d::network::flushDataCallback& flushCallback, const CommCallback& callback);

protected:
    SMComm();
    virtual ~SMComm();

private:
    std::mutex _mutex;

    // 다른넘은 다 받는 listener가 있는데 init정보 같은경우 app 실행시 호출되기 때문에 listener가 없을 수 있다.
    // 그래서 내부에서 만들어줌.
    cocos2d::Node * _initNode;
};

#endif /* SMComm_h */
