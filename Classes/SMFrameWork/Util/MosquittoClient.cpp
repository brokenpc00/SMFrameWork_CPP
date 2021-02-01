//
//  MosquittoClient.cpp
//  IPCT
//
//  Created by SteveKim on 2020/03/04.
//

#include "MosquittoClient.h"
#include "libmosquitto/mosquitto.h"
#include "../../Classes/SMFrameWork/Util/encrypt/AES.h"

#include <sstream>

bool MosquittoClient::_init = false;

static void on_connect(void *ptr, int rc)
{
    MosquittoClient* client = (MosquittoClient*)ptr;
    if(client->getMQTTProtocol()!=NULL){
        client->getMQTTProtocol()->didConnect(rc);
    }
}

static void on_disconnect(void *ptr)
{
    MosquittoClient* client = (MosquittoClient*)ptr;
    if(client->getMQTTProtocol()!=NULL){
        client->getMQTTProtocol()->didDisconnect();
    }
}

static void on_publish(void *ptr, uint16_t message_id)
{
    MosquittoClient* client = (MosquittoClient*)ptr;
    if(client->getMQTTProtocol()!=NULL){
        client->getMQTTProtocol()->didPublish(message_id);
    }
}

static void on_message(void *ptr, const struct mosquitto_message *message)
{
    MosquittoClient* client = (MosquittoClient*)ptr;
    if(client->getMQTTProtocol()!=NULL){
        std::string topic(message->topic);
        std::string payload((char*)message->payload, message->payloadlen);
        std::string getmsg = aes_decryt(payload);
        // for test
        if (getmsg=="") {
            getmsg = payload;
        }
        client->getMQTTProtocol()->didReceiveMessage(getmsg, topic);
    }
}

static void on_subscribe(void *ptr, uint16_t message_id, int qos_count, const uint8_t *granted_qos)
{
    MosquittoClient* client = (MosquittoClient*)ptr;
    if(client->getMQTTProtocol()!=NULL){
        //TODO: implement this
        client->getMQTTProtocol()->didSubscribe(message_id, nullptr);
    }
}

static void on_unsubscribe(void *ptr, uint16_t message_id)
{
    MosquittoClient* client = (MosquittoClient*)ptr;
    if(client->getMQTTProtocol()!=NULL){
        //TODO: implement this
        client->getMQTTProtocol()->didUnsubscribe(message_id);
    }
}


MosquittoClient::MosquittoClient(std::string clientId){
    if(!MosquittoClient::_init){
        mosquitto_lib_init();
        MosquittoClient::_init=true;
    }

    mosq = mosquitto_new(clientId.c_str(), this);
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_disconnect_callback_set(mosq, on_disconnect);
    mosquitto_publish_callback_set(mosq, on_publish);
    mosquitto_message_callback_set(mosq, on_message);
    mosquitto_subscribe_callback_set(mosq, on_subscribe);
    mosquitto_unsubscribe_callback_set(mosq, on_unsubscribe);
}

std::string MosquittoClient::version() {
    int major, minor, revision;
    std::stringstream out;

    mosquitto_lib_version(&major, &minor, &revision);

    out<<major<<"."<<minor<<"."<<revision;

    return out.str();
}

MosquittoClient::~MosquittoClient(){
    if (mosq) {
        mosquitto_destroy(mosq);
        mosq = NULL;
    }

    this->stopAllActions();
//    this->unscheduleAllSelectors();

    //[super dealloc]
}

MosquittoClient* MosquittoClient::create(std::string clientId, MQTTProtocol* caller){
    MosquittoClient* salida = new (std::nothrow) MosquittoClient(clientId);
    if (salida) {
        salida->setMQTTProtocol(caller);
        salida->setHost(std::string(""));
        salida->setPort(1883);
        salida->setKeepAlive(30);
        salida->setCleanSession(true);
        salida->autorelease();
    }
    
    return salida;
}

void MosquittoClient::setLogPriorities(int priorities, int destinations){
    mosquitto_log_init(mosq, priorities, destinations);
}

void MosquittoClient::connect(){
    //std::string host = gethost();
    char *cstrHost=(char*)malloc(getHost().size());
    strcpy(cstrHost, getHost().c_str());
    //const char *cstrHost = gethost().c_str();
    const char *cstrUsername = NULL, *cstrPassword = NULL;

    if (getUserName() != "")
        cstrUsername = getUserName().c_str();

    if (getPassword() != "")
        cstrPassword = getPassword().c_str();

    // FIXME: check for errors
    mosquitto_username_pw_set(mosq, cstrUsername, cstrPassword);

    mosquitto_connect(mosq, cstrHost, _port, _keepAlive, _cleanSession);

    // Setup timer to handle network events
    // FIXME: better way to do this - hook into iOS Run Loop select() ?
    // or run in seperate thread?
    this->schedule(schedule_selector(MosquittoClient::loop),0.01);
    
}

void MosquittoClient::loop(float dt){
    mosquitto_loop(mosq, 0);
}

void MosquittoClient::connectToHost(std::string host) {
    setHost(host);
    connect();
}

void MosquittoClient::reconnect() {
    mosquitto_reconnect(mosq);
}

void MosquittoClient::disconnect() {
    mosquitto_disconnect(mosq);
}

// FIXME: add QoS parameter?
void MosquittoClient::publishString(std::string payload, std::string topic, bool retain){
    // retain : 마지막으로 발송된 메시지를 Broker가 보관
    std::string sendMsg = aes_encryt(payload);
    mosquitto_publish(mosq, NULL, topic.c_str(), sendMsg.size(), (const uint8_t*)sendMsg.c_str(), 0, retain);
}

void MosquittoClient::subscribe(std::string topic){
    subscribe(topic, 0);
}

void MosquittoClient::subscribe(std::string topic, int qos){
    mosquitto_subscribe(mosq, NULL, topic.c_str(), qos);
}

void MosquittoClient::unsubscribe(std::string topic){
    mosquitto_unsubscribe(mosq, NULL, topic.c_str());
}

void MosquittoClient::setMessageRetry(unsigned int seconds){
    mosquitto_message_retry_set(mosq, seconds);
}
