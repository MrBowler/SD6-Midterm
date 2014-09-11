#include "World.hpp"
#include <queue>
#include "../Engine/Time.hpp"
#include "../Engine/DeveloperConsole.hpp"
#include "../Engine/NewMacroDef.hpp"

//-----------------------------------------------------------------------------------------------
World::World( float worldWidth, float worldHeight )
	: m_size( worldWidth, worldHeight )
	, m_playerTexture( nullptr )
	, m_isConnectedToServer( false )
	, m_nextPacketNumber( 0 )
{

}


//-----------------------------------------------------------------------------------------------
void World::Initialize()
{
	InitializeTime();
	m_client.ConnectToServer( IP_ADDRESS, PORT_NUMBER );
	m_playerTexture = Texture::CreateOrGetTexture( PLAYER_TEXTURE_FILE_PATH );
	m_secondsSinceLastInitSend = GetCurrentTimeSeconds();

	m_mainPlayer = new Player;
	m_players.push_back( m_mainPlayer );
	SendJoinGamePacket();
}


//-----------------------------------------------------------------------------------------------
void World::Destruct()
{
	m_client.DisconnectFromServer();
}


//-----------------------------------------------------------------------------------------------
void World::ChangeIPAddress( const std::string& ipAddrString )
{
	unsigned short currentServerPortNumber = m_client.GetServerPortNumber();
	m_client.DisconnectFromServer();
	m_client.ConnectToServer( ipAddrString, currentServerPortNumber );

	m_isConnectedToServer = false;
}


//-----------------------------------------------------------------------------------------------
void World::ChangePortNumber( unsigned short portNumber )
{
	std::string currentServerIPAddress = m_client.GetServerIPAddress();
	m_client.DisconnectFromServer();
	m_client.ConnectToServer( currentServerIPAddress, portNumber );

	m_isConnectedToServer = false;
}


//-----------------------------------------------------------------------------------------------
void World::Update( float deltaSeconds, const Keyboard& keyboard, const Mouse& mouse )
{
	UpdateFromInput( keyboard, mouse );
	ReceivePackets();
	SendUpdate();
}


//-----------------------------------------------------------------------------------------------
void World::RenderObjects3D()
{
	
}


//-----------------------------------------------------------------------------------------------
void World::RenderObjects2D()
{
	
}


//-----------------------------------------------------------------------------------------------
void World::SendPacket( const CS6Packet& packet, bool requireAck )
{
	m_client.SendPacketToServer( (const char*) &packet, sizeof( packet ) );
	++m_nextPacketNumber;

	if( requireAck )
	{
		m_sentPackets.push_back( packet );
	}
}


//-----------------------------------------------------------------------------------------------
void World::SendJoinGamePacket()
{
	CS6Packet ackPacket;
	ackPacket.packetNumber = m_nextPacketNumber;
	ackPacket.packetType = TYPE_Acknowledge;
	ackPacket.timestamp = GetCurrentTimeSeconds();
	ackPacket.data.acknowledged.packetType = TYPE_Acknowledge;

	SendPacket( ackPacket, false );
}


//-----------------------------------------------------------------------------------------------
void World::ResetGame( const CS6Packet& resetPacket )
{
	m_isConnectedToServer = true;

	m_mainPlayer->m_color.r = resetPacket.data.reset.playerColorAndID[0];
	m_mainPlayer->m_color.g = resetPacket.data.reset.playerColorAndID[1];
	m_mainPlayer->m_color.b = resetPacket.data.reset.playerColorAndID[2];
	m_mainPlayer->m_currentPosition.x = resetPacket.data.reset.playerXPosition;
	m_mainPlayer->m_currentPosition.y = resetPacket.data.reset.playerYPosition;
	m_mainPlayer->m_gotoPosition = m_mainPlayer->m_currentPosition;
	m_mainPlayer->m_currentVelocity = Vector2( 0.f, 0.f );
	m_mainPlayer->m_orientationDegrees = 0.f;
	m_mainPlayer->m_isIt = resetPacket.data.reset.isIt;

	CS6Packet ackPacket;
	ackPacket.packetNumber = m_nextPacketNumber;
	ackPacket.packetType = TYPE_Acknowledge;
	ackPacket.playerColorAndID[0] = m_mainPlayer->m_color.r;
	ackPacket.playerColorAndID[1] = m_mainPlayer->m_color.g;
	ackPacket.playerColorAndID[2] = m_mainPlayer->m_color.b;
	ackPacket.timestamp = GetCurrentTimeSeconds();
	ackPacket.data.acknowledged.packetNumber = resetPacket.packetNumber;
	ackPacket.data.acknowledged.packetType = TYPE_Reset;

	SendPacket( ackPacket, false );
}


//-----------------------------------------------------------------------------------------------
void World::UpdatePlayer( const CS6Packet& updatePacket )
{
	for( unsigned int playerIndex = 0; playerIndex < m_players.size(); ++playerIndex )
	{
		Player* player = m_players[ playerIndex ];
		if( player->m_color.r == updatePacket.playerColorAndID[0]
			&& player->m_color.g == updatePacket.playerColorAndID[1]
			&& player->m_color.b == updatePacket.playerColorAndID[2] )
		{
			player->m_gotoPosition.x = updatePacket.data.updated.xPosition;
			player->m_gotoPosition.y = updatePacket.data.updated.yPosition;
			player->m_currentVelocity.x = updatePacket.data.updated.xVelocity;
			player->m_currentVelocity.y = updatePacket.data.updated.yVelocity;
			player->m_orientationDegrees = updatePacket.data.updated.yawDegrees;
			player->m_isIt;
			player->m_secondsSinceLastUpdate = 0.f;

			return;
		}
	}

	Player* player = new Player();
	player->m_color.r = updatePacket.playerColorAndID[0];
	player->m_color.g = updatePacket.playerColorAndID[1];
	player->m_color.b = updatePacket.playerColorAndID[2];
	player->m_currentPosition.x = updatePacket.data.updated.xPosition;
	player->m_currentPosition.y = updatePacket.data.updated.yPosition;
	player->m_gotoPosition = player->m_currentPosition;
	player->m_currentVelocity.x = updatePacket.data.updated.xVelocity;
	player->m_currentVelocity.y = updatePacket.data.updated.yVelocity;
	player->m_orientationDegrees = updatePacket.data.updated.yawDegrees;
	player->m_isIt;
	player->m_secondsSinceLastUpdate = 0.f;

	m_players.push_back( player );
}


//-----------------------------------------------------------------------------------------------
void World::UpdateFromInput( const Keyboard& keyboard, const Mouse& )
{
	if( g_developerConsole.m_drawConsole )
		return;
}


//-----------------------------------------------------------------------------------------------
void World::SendUpdate()
{
	if( !m_isConnectedToServer && ( GetCurrentTimeSeconds() - m_secondsSinceLastInitSend ) > SECONDS_BEFORE_RESEND_INIT_PACKET )
	{
		SendJoinGamePacket();
		m_secondsSinceLastInitSend = GetCurrentTimeSeconds();
		return;
	}
}


//-----------------------------------------------------------------------------------------------
void World::ReceivePackets()
{
	CS6Packet packet;

	while( m_client.ReceivePacketFromServer( (char*) &packet, sizeof( packet ) ) )
	{
		if( packet.packetType == TYPE_Update )
		{
			UpdatePlayer( packet );
		}
		else if( packet.packetType == TYPE_Reset )
		{
			ResetGame( packet );
		}
	}
}