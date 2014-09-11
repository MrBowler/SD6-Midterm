#include "GameServer.hpp"


//-----------------------------------------------------------------------------------------------
void GameServer::Initalize()
{
	srand( (unsigned int) time( NULL ) );

	InitializeTime();
	m_server.StartServer( PORT_NUMBER );
	m_nextPacketNumber = 0;

	std::cout << "Server is up and running\n";
}


//-----------------------------------------------------------------------------------------------
void GameServer::Update()
{
	GetPackets();
	SendUpdatesToClients();
}


//-----------------------------------------------------------------------------------------------
void GameServer::SendPacketToClient( const CS6Packet& pkt, const ClientInfo& info, bool requireAck )
{
	struct sockaddr_in clientAddr;
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = inet_addr( info.m_ipAddress );
	clientAddr.sin_port = info.m_portNumber;
	m_server.SendPacketToClient( (const char*) &pkt, sizeof( pkt ), clientAddr );
	++m_nextPacketNumber;

	if( requireAck )
	{
		std::vector< CS6Packet > sentPackets;
		std::map< ClientInfo, std::vector< CS6Packet > >::iterator vecIter = m_sendPacketsPerClient.find( info );
		if( vecIter != m_sendPacketsPerClient.end() )
		{
			sentPackets = vecIter->second;
		}

		sentPackets.push_back( pkt );
		m_sendPacketsPerClient[ info ] = sentPackets;
	}
}


//-----------------------------------------------------------------------------------------------
void GameServer::SendPacketToAllClients( const CS6Packet& pkt, bool requireAck )
{
	std::map< ClientInfo, Player* >::iterator playerIter;
	for( playerIter = m_players.begin(); playerIter != m_players.end(); ++playerIter )
	{
		SendPacketToClient( pkt, playerIter->first, requireAck );
	}
}


//-----------------------------------------------------------------------------------------------
std::string GameServer::ConvertNumberToString( int number )
{
	return static_cast< std::ostringstream* >( &( std::ostringstream() << number ) )->str();
}


//-----------------------------------------------------------------------------------------------
Color3b GameServer::GetPlayerColorForID( unsigned int playerID )
{
	if( playerID == 0 )
		return Color3b( 255, 0, 0 );
	if( playerID == 1 )
		return Color3b( 0, 255, 0 );
	if( playerID == 2 )
		return Color3b( 0, 0, 255 );
	if( playerID == 3 )
		return Color3b( 255, 255, 0 );
	if( playerID == 4 )
		return Color3b( 255, 0, 255 );
	if( playerID == 5 )
		return Color3b( 0, 255, 255 );
	if( playerID == 6 )
		return Color3b( 255, 165, 0 );
	if( playerID == 7 )
		return Color3b( 128, 0, 128 );

	return Color3b( 255, 255, 255 );
}


//-----------------------------------------------------------------------------------------------
Vector2 GameServer::GetPlayerPosition( const Color3b& playerID )
{
	if( playerID == GetPlayerColorForID( 0 ) )
		return Vector2( 0.f, 0.f );
	if( playerID == GetPlayerColorForID( 1 ) )
		return Vector2( 0.f, MAP_SIZE_HEIGHT - 1.f );
	if( playerID == GetPlayerColorForID( 2 ) )
		return Vector2( MAP_SIZE_WIDTH - 1.f, MAP_SIZE_HEIGHT - 1.f );
	if( playerID == GetPlayerColorForID( 3 ) )
		return Vector2( MAP_SIZE_WIDTH - 1.f, 0.f );
	if( playerID == GetPlayerColorForID( 4 ) )
		return Vector2( 0.f, MAP_SIZE_HEIGHT * 0.5f );
	if( playerID == GetPlayerColorForID( 5 ) )
		return Vector2( MAP_SIZE_WIDTH - 1.f, MAP_SIZE_HEIGHT * 0.5f );
	if( playerID == GetPlayerColorForID( 6 ) )
		return Vector2( MAP_SIZE_WIDTH * 0.5f, 0.f );
	if( playerID == GetPlayerColorForID( 7 ) )
		return Vector2( MAP_SIZE_WIDTH * 0.5f, MAP_SIZE_HEIGHT - 1.f );

	return Vector2( MAP_SIZE_WIDTH * 0.5f, MAP_SIZE_HEIGHT * 0.5f );
}


//-----------------------------------------------------------------------------------------------
Vector2 GameServer::GetRandomPosition()
{
	Vector2 returnVec;
	returnVec.x = (float) ( rand() % MAP_SIZE_WIDTH );
	returnVec.y = (float) ( rand() % MAP_SIZE_HEIGHT );

	return returnVec;
}


//-----------------------------------------------------------------------------------------------
bool GameServer::HasItPlayerBeenAssigned()
{
	std::map< ClientInfo, Player* >::iterator playerIter;
	for( playerIter = m_players.begin(); playerIter != m_players.end(); ++playerIter )
	{
		Player* player = playerIter->second;
		if( player->m_isIt )
			return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void GameServer::AddPlayer( const ClientInfo& info )
{
	Player* player;

	std::map< ClientInfo, Player* >::iterator playerIter = m_players.find( info );
	if( playerIter == m_players.end() )
	{
		player = new Player();
	}
	else
	{
		player = playerIter->second;
	}

	player->m_color = GetPlayerColorForID( m_players.size() );
	player->m_isIt = !HasItPlayerBeenAssigned();
	player->m_position = GetPlayerPosition( player->m_color );
	player->m_velocity = Vector2( 0.f, 0.f );
	player->m_orientationDegrees = 0.f;
	player->m_lastUpdateTime = GetCurrentTimeSeconds();

	m_players[ info ] = player;

	CS6Packet resetPacket;
	resetPacket.packetNumber = m_nextPacketNumber;
	resetPacket.packetType = TYPE_Reset;
	resetPacket.playerColorAndID[0] = player->m_color.r;
	resetPacket.playerColorAndID[1] = player->m_color.g;
	resetPacket.playerColorAndID[2] = player->m_color.b;
	resetPacket.timestamp = GetCurrentTimeSeconds();
	resetPacket.data.reset.playerXPosition = player->m_position.x;
	resetPacket.data.reset.playerYPosition = player->m_position.y;
	resetPacket.data.reset.playerColorAndID[0] = player->m_color.r;
	resetPacket.data.reset.playerColorAndID[1] = player->m_color.g;
	resetPacket.data.reset.playerColorAndID[2] = player->m_color.b;
	resetPacket.data.reset.isIt = player->m_isIt;

	SendPacketToClient( resetPacket, info, true );
}


//-----------------------------------------------------------------------------------------------
void GameServer::UpdatePlayer( const CS6Packet& updatePacket, const ClientInfo& info )
{
	std::map< ClientInfo, Player* >::iterator playerIter = m_players.find( info );
	if( playerIter != m_players.end() )
	{
		Player* player = playerIter->second;
		player->m_position.x = updatePacket.data.updated.xPosition;
		player->m_position.y = updatePacket.data.updated.yPosition;
		player->m_velocity.x = updatePacket.data.updated.xVelocity;
		player->m_velocity.y = updatePacket.data.updated.yVelocity;
		player->m_orientationDegrees = updatePacket.data.updated.yawDegrees;
	}
}


//-----------------------------------------------------------------------------------------------
void GameServer::ProcessAckPackets( const CS6Packet& ackPacket, const ClientInfo& info )
{
	if( ackPacket.data.acknowledged.packetType == TYPE_Acknowledge )
	{
		AddPlayer( info );
	}

	std::map< ClientInfo, std::vector< CS6Packet > >::iterator vecIter = m_sendPacketsPerClient.find( info );
	if( vecIter != m_sendPacketsPerClient.end() )
	{
		std::vector< CS6Packet > sentPackets = vecIter->second;
		for( unsigned int packetIndex = 0; packetIndex < sentPackets.size(); ++packetIndex )
		{
			CS6Packet packet = sentPackets[ packetIndex ];
			if( packet.packetNumber == ackPacket.data.acknowledged.packetNumber )
			{
				sentPackets.erase( sentPackets.begin() + packetIndex );
				m_sendPacketsPerClient[ info ] = sentPackets;
				break;
			}

			SendPacketToClient( packet, info, false );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void GameServer::PlayerTag( const CS6Packet& tagPacket, const ClientInfo& info )
{
	CS6Packet ackPacket;
	ackPacket.packetNumber = m_nextPacketNumber;
	ackPacket.packetType = TYPE_Acknowledge;
	ackPacket.playerColorAndID[0] = tagPacket.playerColorAndID[0];
	ackPacket.playerColorAndID[1] = tagPacket.playerColorAndID[1];
	ackPacket.playerColorAndID[2] = tagPacket.playerColorAndID[2];
	ackPacket.timestamp = GetCurrentTimeSeconds();
	ackPacket.data.acknowledged.packetNumber = tagPacket.packetNumber;
	ackPacket.data.acknowledged.packetType = TYPE_Tag;

	SendPacketToClient( tagPacket, info, false );

	Color3b taggedPlayerColor( tagPacket.data.playerTagged.playerColorAndID[0], tagPacket.data.playerTagged.playerColorAndID[1], tagPacket.data.playerTagged.playerColorAndID[2] );

	std::map< ClientInfo, Player* >::iterator playerIter;
	for( playerIter = m_players.begin(); playerIter != m_players.end(); ++playerIter )
	{
		Player* player = playerIter->second;
		Vector2 resetPos = GetPlayerPosition( player->m_color );
		CS6Packet resetPacket;
		resetPacket.packetNumber = m_nextPacketNumber;
		resetPacket.packetType = TYPE_Reset;
		resetPacket.playerColorAndID[0] = player->m_color.r;
		resetPacket.playerColorAndID[1] = player->m_color.g;
		resetPacket.playerColorAndID[2] = player->m_color.b;
		resetPacket.timestamp = GetCurrentTimeSeconds();
		resetPacket.data.reset.playerXPosition = resetPos.x;
		resetPacket.data.reset.playerYPosition = resetPos.y;
		if( taggedPlayerColor == player->m_color )
			resetPacket.data.reset.isIt = true;
		else
			resetPacket.data.reset.isIt = false;

		SendPacketToClient( resetPacket, info, true );
	}
}


//-----------------------------------------------------------------------------------------------
void GameServer::SendUpdatesToClients()
{
	std::map< ClientInfo, Player* >::iterator playerIter;
	for( playerIter = m_players.begin(); playerIter != m_players.end(); ++playerIter )
	{
		Player* player = playerIter->second;
		CS6Packet updatePacket;
		updatePacket.packetNumber = m_nextPacketNumber;
		updatePacket.packetType = TYPE_Update;
		updatePacket.playerColorAndID[0] = player->m_color.r;
		updatePacket.playerColorAndID[1] = player->m_color.g;
		updatePacket.playerColorAndID[2] = player->m_color.b;
		updatePacket.timestamp = GetCurrentTimeSeconds();
		updatePacket.data.updated.xPosition = player->m_position.x;
		updatePacket.data.updated.yPosition = player->m_position.y;
		updatePacket.data.updated.xVelocity = player->m_velocity.x;
		updatePacket.data.updated.yVelocity = player->m_velocity.y;
		updatePacket.data.updated.yawDegrees = player->m_orientationDegrees;
		updatePacket.data.updated.isIt = player->m_isIt;

		SendPacketToAllClients( updatePacket, false );
	}
}


//-----------------------------------------------------------------------------------------------
void GameServer::GetPackets()
{
	CS6Packet packet;
	struct sockaddr_in clientAddr;
	clientAddr.sin_family = AF_INET;
	int clientLen = sizeof( clientAddr );

	while( m_server.ReceivePacketFromClient( (char*) &packet, sizeof( packet ), clientAddr, clientLen ) )
	{
		ClientInfo info;
		info.m_ipAddress = inet_ntoa( clientAddr.sin_addr );
		info.m_portNumber = clientAddr.sin_port;

		if( packet.packetType == TYPE_Acknowledge )
		{
			ProcessAckPackets( packet, info );
		}
		else if( packet.packetType == TYPE_Update )
		{
			UpdatePlayer( packet, info );
		}
		else if( packet.packetType == TYPE_Tag )
		{
			PlayerTag( packet, info );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void GameServer::ResendAckPackets()
{
	std::map< ClientInfo, std::vector< CS6Packet > >::iterator vecIter;
	for( vecIter = m_sendPacketsPerClient.begin(); vecIter != m_sendPacketsPerClient.end(); ++vecIter )
	{
		std::vector< CS6Packet > sentPackets = vecIter->second;
		for( unsigned int packetIndex = 0; packetIndex < sentPackets.size(); ++packetIndex )
		{
			CS6Packet packet = sentPackets[ packetIndex ];
			if( ( GetCurrentTimeSeconds() - packet.timestamp ) > SECONDS_BEFORE_RESEND_RELIABLE_PACKETS )
			{
				SendPacketToClient( packet, vecIter->first, false );
			}
		}
	}
}