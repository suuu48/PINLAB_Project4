#include "text_h1.h"

void SendStringToClient(int _socket, const char *_ccp);
const char *itoa(unsigned int _uiData);

int main()
{
    MESSAGE m_message;

    fd_set status;

    int servSock;          
    int clntSock[MAXUSER]; 
    int tempSock;

    int iMaxSock; //

    unsigned int InClient;

    int iCount;
    int i;

    struct sockaddr_in echoServAddr; //  Server's Socket structure
    struct sockaddr_in echoClntAddr; //  Client's Addr

    unsigned short echoServPort;
    unsigned int clntLen;

    time_t stTempTime;

    int iRet;

    unsigned char Clientmsg[500];

    echoServPort = 9999; // port (type: unsigned short -> cant change)

    servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // Create Socket

    if (servSock < 0)
    {
        printf("Socket Function Error!\n");
        return (0);
    }

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = PF_INET;
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    echoServAddr.sin_port = htons(echoServPort);

    iRet = bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr));

    if (iRet < 0)
    {
        close(servSock);
        printf("Bind Failed Error!\n");
        return (0);
    }

    iRet = listen(servSock, MAXPENDING); // MAX PENDING : 5

    if (iRet < 0)
    {
        close(servSock);
        printf("Listen  Failed Error!\n");
        return (0);
    }

    clntLen = sizeof(echoClntAddr);

    iMaxSock = servSock + 1;

    InClient = 0;

    while (1)
    {
        FD_ZERO(&status);
        FD_SET(0, &status); 
        FD_SET(servSock, &status);

        for (i = 0; i < InClient; i++)
        {
            FD_SET(clntSock[i], &status);

            if (iMaxSock <= clntSock[i])
            {
                iMaxSock = clntSock[i] + 1;
            }
        }

        iRet = select(iMaxSock, &status, 0, 0, 0);
        if (iRet < 0)
        {
            printf("Select Function Error!\n");

            strcpy(m_message.m_buffer, "** Server Error **");

            for (iCount = 0; iCount < InClient; iCount++) 
            {
                write(clntSock[iCount], &m_message, sizeof(m_message));
            }

            // strcpy(m_message.m_buffer, "/q");
            strcpy(m_message.m_buffer, "** Quit **");

            for (iCount = 0; iCount < InClient; iCount++) 
            {
                write(clntSock[iCount], &m_message, sizeof(m_message));
            }

            break;
        }

        // New Client Come in
        if (1 == FD_ISSET(servSock, &status))
        {
            tempSock = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntLen);

            if (tempSock < 0)
            {
                printf("Accept Function Error!\n");

                continue;
            }

            printf("New Client IP : %s\n", inet_ntoa(echoClntAddr.sin_addr));

            // !< MAXUSER
            if (MAXUSER <= InClient)
            {
                close(tempSock);

                continue;
            }

            // < MAXUSER
            clntSock[InClient] = tempSock;
            InClient = InClient + 1;

            // Print Clients num
            printf("Number of Clients: %d\n", InClient);
        }

        // Normal
        else if (1 == FD_ISSET(0, &status))
        {

            iRet = read(0, m_message.m_buffer, sizeof(m_message.m_buffer));
            m_message.m_buffer[iRet - 1] = 0; // exception null
            strcpy(m_message.m_userName, "Server");

            time(&stTempTime);
            strftime(m_message.m_time, 26, "%H:%M", localtime(&stTempTime));

            // Server Quit
            /* if (m_message.m_buffer[0] == CLTEND) */
            if (0 == strcmp(LOGOUT, m_message.m_buffer))
            {
                strcpy(m_message.m_buffer, ENDMSG);

                // Send MSG for All Clients
                for (iCount = 0; iCount < InClient; iCount++)
                {
                    write(clntSock[iCount], &m_message, sizeof(m_message));
                }
                break;
            }

            // Send Server's MSG for All Clients
            for (iCount = 0; iCount < InClient; iCount++)
            {
                write(clntSock[iCount], &m_message, sizeof(m_message));
            }
        }

        // Read Client's Message
        else
        {
            // Send MSG for All Clients
            for (iCount = 0; iCount < InClient; iCount++)
            {
                if (1 == FD_ISSET(clntSock[iCount], &status))
                {
                    // iRet = len(Client's MSG)
                    iRet = read(clntSock[iCount], &m_message, sizeof(Clientmsg) - 1);

                    if (iRet != 0)
                    {
                        // Client quit (cmp = compare)
                        // if Client's MSG == LOGOUT
                        if (0 == strcmp(LOGOUT, m_message.m_buffer))
                        {
                            printf("** Some Client Quit **\n");

                            InClient = InClient - 1;
                            
                            for (i = 0; i < InClient; i++)
                            {
                                write(clntSock[i], &m_message, sizeof(m_message));
                            }

                            printf("\n");
                            continue;
                        }

                        // macro
                        else if ('/' == m_message.m_buffer[0])
                        {
                            switch (m_message.m_buffer[1])
                            {
                            case 'h':
                                strcpy(m_message.m_userName, "Server");
                                strcpy(m_message.m_buffer, "help\n\t-- p : Server Port\n\t-- i : Server IP\n");
                                write(clntSock[iCount], &m_message, sizeof(m_message));
                                break;

                            case 'p':
                                strcpy(m_message.m_userName, "Server");
                                strcpy(m_message.m_buffer, itoa(ntohs(echoClntAddr.sin_port)));
                                write(clntSock[iCount], &m_message, sizeof(m_message));
                                break;

                            case 'i':
                                strcpy(m_message.m_userName, "Server");
                                strcpy(m_message.m_buffer, inet_ntoa(echoClntAddr.sin_addr));
                                write(clntSock[iCount], &m_message, sizeof(m_message));
                                break;

                            default:
                                strcpy(m_message.m_userName, "Server");
                                strcpy(m_message.m_buffer, ": Invalid command");
                                write(clntSock[iCount], &m_message, sizeof(m_message));
                                break;
                            }
                        }

                        // Send Client's MSG
                        else
                        {
                            printf("%s: ", m_message.m_userName);
                            fflush(stdout);
                            printf("%s", m_message.m_buffer); 
                            printf("(%s)\n", m_message.m_time);

                            for (i = 0; i < InClient; i++)
                            {
                                write(clntSock[i], &m_message, sizeof(m_message));
                            }
                        }
                    }
                }
            }
        }
    }

    printf("** Quit Server ** \n");
   
    for (i = 0; i < InClient; i++) 
    {
        close(clntSock[i]); 
    }

    close(servSock); 

    return (0); 
}

void SendStringToClient(int _socket, const char *_ccp)
{
    write(_socket, _ccp, strlen(_ccp));
    return;
}

const char *itoa(unsigned int _uiData)
{
    char cBuffer[5] = "0000";
    int i;
    int j;

    for (i = 10000, j = 0; i >= 0; i /= 10, j++)
    {
        cBuffer[j] = _uiData / i;
        _uiData = i * (_uiData / i);
    }
    return cBuffer;
}
