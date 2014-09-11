#ifndef include_GameServer
#define include_GameServer
#pragma once

//-----------------------------------------------------------------------------------------------
#include <map>
#include <string>
#include <time.h>
#include <vector>
#include <sstream>
#include <iostream>
#include "Player.hpp"
#include "Color3b.hpp"
#include "CS6Packet.hpp"
#include "UDPServer.hpp"
#include "ClientInfo.hpp"
#include "../Engine/Time.hpp"


//-----------------------------------------------------------------------------------------------
const unsigned short PORT_NUMBER = 5000;
const int MAP_SIZE_WIDTH = 500;
const int MAP_SIZE_HEIGHT = 500;
const double SECONDS_BEFORE_SEND_UPDATE = 0.0045;
const double SECONDS_BEFORE_RESEND_RELIABLE_PACKETS = 0.25;


//-----------------------------------------------------------------------------------------------
class GameServer
{
public:
	GameServer() {}
	void Initalize();
	void Update();

private:
	void SendPacketToClient( const CS6Packet& pkt, const ClientInfo& info );
	std::string ConvertNumberToString( int number );
	Color3b GetPlayerColorForID( unsigned int playerID );
	Vector2 GetRandomPosition();
	void GetPackets();

	UDPServer	m_server;
};


#endif // include_GameServer