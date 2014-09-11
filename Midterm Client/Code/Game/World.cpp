#include "World.hpp"
#include <queue>
#include "../Engine/Time.hpp"
#include "../Engine/DeveloperConsole.hpp"
#include "../Engine/NewMacroDef.hpp"

//-----------------------------------------------------------------------------------------------
World::World( float worldWidth, float worldHeight )
	: m_size( worldWidth, worldHeight )
	, m_isConnectedToServer( false )
{

}


//-----------------------------------------------------------------------------------------------
void World::Initialize()
{
	InitializeTime();
	m_client.ConnectToServer( IP_ADDRESS, PORT_NUMBER );
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
void World::SendPacket( const CS6Packet& packet )
{
	m_client.SendPacketToServer( (const char*) &packet, sizeof( packet ) );
}


//-----------------------------------------------------------------------------------------------
void World::UpdateFromInput( const Keyboard& keyboard, const Mouse& )
{
	if( g_developerConsole.m_drawConsole )
		return;
}


//-----------------------------------------------------------------------------------------------
void World::ReceivePackets()
{
	CS6Packet packet;

	while( m_client.ReceivePacketFromServer( (char*) &packet, sizeof( packet ) ) )
	{
		
	}
}