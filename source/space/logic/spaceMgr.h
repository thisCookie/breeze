﻿/*
* breeze License
* Copyright (C) 2015 - 2016 YaweiZhang <yawei.zhang@foxmail.com>.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/





#ifndef _SpaceMgr_H_
#define _SpaceMgr_H_
#include <common.h>
#include <ProtoCommon.h>
#include <ProtoDocker.h>
#include <ProtoSpaceCommon.h>
#include <ProtoSpaceClient.h>
#include <rvo2/RVO.h>
#include "space.h"



class SpaceMgr : public Singleton<SpaceMgr>
{
public:
    SpaceMgr();
    bool init(const std::string & configName, ui32 serverID);
    bool start();
    void stop();
    void forceStop();
    void onShutdown();
    bool run();

    
public:
    bool isStopping();
public:
    void sendToSession(SessionID sessionID, const char * block, unsigned int len);
    template<class Proto>
    void sendToSession(SessionID sessionID, const Proto & proto);

    template<class Proto>
    void sendToWorld(const Proto & proto);
private:
    //内部接口 
    //打开监听端口,新连接 
    bool startClientListen();
    bool startWorldConnect();
    bool loadSpaces();
    void onTimer();
public:
    SpacePtr getSpace(SpaceID);
    void refreshSpaceStatusToWorld(SpaceID spaceID);
private:
    //docker间通讯处理 
    void event_onWorldLinked(TcpSessionPtr session);
    void event_onWorldClosed(TcpSessionPtr session);
    void event_onWorldMessage(TcpSessionPtr   session, const char * begin, unsigned int len);

private:
    //客户端通讯处理 
    void event_onClientLinked(TcpSessionPtr session);
    void event_onClientPulse(TcpSessionPtr session);
    void event_onClientClosed(TcpSessionPtr session);
    void event_onClientMessage(TcpSessionPtr   session, const char * begin, unsigned int len);

private:
    std::map<SpaceID, SpacePtr> _spaces;
    std::map<SpaceID, SpacePtr> _actives;
    std::map<SpaceID, SpacePtr> _frees;

    SessionID _worldSessionID = InvalidSessionID;
    AccepterID _clientListen = InvalidAccepterID;
};




template<class Proto>
void SpaceMgr::sendToSession(SessionID sessionID, const Proto & proto)
{
    try
    {
        WriteStream ws(Proto::getProtoID());
        ws << proto;
        SessionManager::getRef().sendSessionData(sessionID, ws.getStream(), ws.getStreamLen());
    }
    catch (const std::exception & e)
    {
        LOGE("Docker::sendToSession catch except error. e=" << e.what());
    }
}
template<class Proto>
void SpaceMgr::sendToWorld(const Proto & proto)
{
    if (_worldSessionID != InvalidSessionID)
    {
        sendToSession(_worldSessionID, proto);
    }
}


#endif
