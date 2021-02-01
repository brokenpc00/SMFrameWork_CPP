//
//  MosquittoClient.h
//  IPCT
//
//  Created by SteveKim on 2020/03/04.
//

#ifndef MosquittoClient_h
#define MosquittoClient_h

#include <cocos2d.h>
#include <vector>

class MQTTProtocol
{
public:
    virtual void didConnect(int code) {};
    virtual void didDisconnect() {};
    
    virtual void didPublish(int messageId) {};
    
    virtual void didReceiveMessage(std::string message, std::string topic) {};
    
    virtual void didSubscribe(int messageID, std::vector<int> * grantedQos) {};
    virtual void didUnsubscribe(int messageID) {};
};

class MosquittoClient : public cocos2d::Node
{
public:
    void setHost(std::string host) {_host=host;}
    void setPort(uint16_t port) {_port = port;}
    void setUserName(std::string username) {_username=username;};
    void setPassword(std::string password) {_password=password;};
    void setKeepAlive(uint16_t keepAlive){_keepAlive=keepAlive;};
    void setCleanSession(bool cleanSession) {_cleanSession=cleanSession;};
    void setMQTTProtocol(MQTTProtocol* protocol) {_protocol=protocol;};
    
    std::string getHost() {return _host;}
    uint16_t getPort() {return _port;}
    std::string getUserName() {return _username;}
    std::string getPassword() {return _password;}
    uint16_t getKeepAlive() {return _keepAlive;}
    bool getClienSession() {return _cleanSession;}
    MQTTProtocol * getMQTTProtocol() {return _protocol;}
    
private:
    static bool _init;
    struct mosquitto * mosq;

protected:
    MosquittoClient(std::string clientId);
    virtual ~MosquittoClient();

public:
    static std::string version();
    
    static MosquittoClient* create(std::string clientId, MQTTProtocol* protocol);

    void setLogPriorities(int priorities, int destinations);
    void setMessageRetry(unsigned int seconds);
    void connect();
    void connectToHost(std::string host);
    void reconnect();
    void disconnect();

    void publishString(std::string payload, std::string topic, bool retain);

    void subscribe(std::string topic);
    void subscribe(std::string topic, int qos);
    void unsubscribe(std::string topic);


    void loop(float dt);
    
private:
    std::string _host;
    uint16_t _port;
    std::string _username;
    std::string _password;
    uint16_t _keepAlive;
    bool _cleanSession;
    MQTTProtocol * _protocol;
};

#endif /* MosquittoClient_h */
