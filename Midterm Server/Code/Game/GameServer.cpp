#include "GameServer.hpp"


//-----------------------------------------------------------------------------------------------
void GameServer::Initalize()
{
	srand( (unsigned int) time( NULL ) );

	InitializeTime();
	m_server.StartServer( PORT_NUMBER );

	std::cout << "Server is up and running\n";
}


//-----------------------------------------------------------------------------------------------
void GameServer::Update()
{
	GetPackets();
}


//-----------------------------------------------------------------------------------------------
void GameServer::SendPacketToClient( const CS6Packet& pkt, const ClientInfo& info )
{
	struct sockaddr_in clientAddr;
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = inet_addr( info.m_ipAddress );
	clientAddr.sin_port = info.m_portNumber;
	m_server.SendPacketToClient( (const char*) &pkt, sizeof( pkt ), clientAddr );
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
Vector2 GameServer::GetRandomPosition()
{
	Vector2 returnVec;
	returnVec.x = (float) ( rand() % MAP_SIZE_WIDTH );
	returnVec.y = (float) ( rand() % MAP_SIZE_HEIGHT );

	return returnVec;
}


//-----------------------------------------------------------------------------------------------
void GameServer::GetPackets()
{
	CS6Packet pkt;
	struct sockaddr_in clientAddr;
	clientAddr.sin_family = AF_INET;
	int clientLen = sizeof( clientAddr );

	while( m_server.ReceivePacketFromClient( (char*) &pkt, sizeof( pkt ), clientAddr, clientLen ) )
	{
		ClientInfo info;
		info.m_ipAddress = inet_ntoa( clientAddr.sin_addr );
		info.m_portNumber = clientAddr.sin_port;
	}
}