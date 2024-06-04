#include "text_h1.h"
#include "text_h2.h"
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[])
{
  fd_set status;
  MESSAGE m_message;
  char m_userName[20];
  
  int sock;
  struct sockaddr_in echoServAddr; 
  unsigned short echoServPort;  
  char *servIP; 
  char *echoString;
  char buffer[500]; 

  time_t stTempTime;
  int iRet;

  servIP = argv[1]; 
  
  sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  
  if( sock < 0 )
  {
    printf( "Socket Function Failed!\n" );

    return 0;
  }

  memset( &echoServAddr, 0, sizeof( echoServAddr ) ); 
  echoServAddr.sin_family = AF_INET;  
  echoServAddr.sin_addr.s_addr = inet_addr( servIP ); 
  echoServAddr.sin_port = htons( 9999 );  

  iRet = connect( sock, ( struct sockaddr* )&echoServAddr, sizeof( echoServAddr ) );
  
  if( iRet < 0 )  
  {
    printf( "Connect Function Failed!\n" );

    close( sock );

    return 0;
  }

  printf( "Name: " );
  fflush( stdout );
  iRet = read( 0, m_userName, sizeof( m_userName ) );
  m_userName[iRet-1] = 0;

  while( 1 )
  {
    FD_ZERO( &status );
    FD_SET( 0, &status );
    FD_SET( sock, &status );
    iRet = select( sock+1, &status, 0, 0, 0 );

    if( iRet < 0 )  
    {
      printf( "Select Function Error!\n" );
      write( sock, LOGOUT, sizeof(LOGOUT) );
      break;
    }
    
    if( 1 == FD_ISSET( 0, &status ) )
    {
      iRet = read( 0, m_message.m_buffer, sizeof( m_message.m_buffer ) );

      time( &stTempTime );
      strftime( m_message.m_time, 26, " %H:%M", localtime( &stTempTime ) );
      
      m_message.m_buffer[iRet-1] = 0;
      
      strcpy( m_message.m_userName, m_userName );

      /* if( CLTEND == m_message.m_buffer[0] ) */
      if (0 == strcmp(LOGOUT, m_message.m_buffer)) 
      {
        printf( "** Quit **\n" );
 
        strcpy( m_message.m_buffer, LOGOUT );
        write( sock, &m_message,sizeof( m_message ) );
        
        break;
      }
       
      write( sock, &m_message, sizeof( m_message ) ); 
    }

    else if( 1 == FD_ISSET( sock, &status ) )
    {
      read( sock, &m_message, sizeof( m_message ) );
      
      if( 0 == strcmp( ENDMSG, m_message.m_buffer ) )  
      {
        printf( "** Server Quit **\n" );
        break;
      }

      printf( "%s: ", m_message.m_userName );
      fflush( stdout ); 
      
      // It dosent have null, So It doesnt print
      printf( "%s", m_message.m_buffer );
      printf( "(%s)\n", m_message.m_time );  
    }
  }
  
  printf("m");

  close( sock );  
  return 0;
}
