
#include <iostream>
#include <sstream>
#include <string>

#define _WIN32_WINNT 0x501

#include <WinSock2.h>
#include <WS2tcpip.h>

// ����������, ����� �������� ����������� � DLL-����������� 
// ��� ������ � �������
#pragma comment(lib, "Ws2_32.lib")

using std::cerr;

int main()
{
    // ��������� ��������� ��� �������� ����������
    // � ���������� Windows Sockets
    WSADATA wsaData;

    // ����� ������������� ���������� ������� ���������
    // (������������ Ws2_32.dll)
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    // ���� ��������� ������ ��������� ����������
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << "\n";
        return result;
    }

    struct addrinfo* addr = NULL; // ���������, �������� ����������
    // �� IP-������  ���������� ������

    // ������ ��� ������������� ��������� ������
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));

    // AF_INET ����������, ��� ������������ ���� ��� ������ � �������
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; // ������ ��������� ��� ������
    hints.ai_protocol = IPPROTO_TCP; // ���������� �������� TCP
    // ����� �������� �� �����, ����� ��������� �������� ����������
    hints.ai_flags = AI_PASSIVE;

    // �������������� ���������, �������� ����� ������ - addr.
    // HTTP-������ ����� ������ �� 8000-� ����� ����������
    result = getaddrinfo("127.0.0.1", "8000", &hints, &addr);
    printf("1.getaddrinfo: %d\n", result);

    // ���� ������������� ��������� ������ ����������� � �������,
    // ������� ���������� �� ���� � �������� ���������� ��������� 
    if (result != 0) {
        cerr << "getaddrinfo failed: " << result << "\n";
        WSACleanup(); // �������� ���������� Ws2_32.dll
        return 1;
    }

    // �������� ������
    int listen_socket = socket(addr->ai_family, addr->ai_socktype,
        addr->ai_protocol);
    printf("2.listen_socket: %d\n", listen_socket);
    // ���� �������� ������ ����������� � �������, ������� ���������,
    // ����������� ������, ���������� ��� ��������� addr,
    // ��������� dll-���������� � ��������� ���������
    if (listen_socket == INVALID_SOCKET) {
        cerr << "Error at socket: " << WSAGetLastError() << "\n";
        freeaddrinfo(addr);
        WSACleanup();
        return 1;
    }

    // ����������� ����� � IP-������
    result = bind(listen_socket, addr->ai_addr, (int)addr->ai_addrlen);
    printf("3.bind: %d\n", result);
    // ���� ��������� ����� � ������ �� �������, �� ������� ���������
    // �� ������, ����������� ������, ���������� ��� ��������� addr.
    // � ��������� �������� �����.
    // ��������� DLL-���������� �� ������ � ��������� ���������.
    if (result == SOCKET_ERROR) {
        cerr << "bind failed with error: " << WSAGetLastError() << "\n";
        freeaddrinfo(addr);
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    printf("4.listen \n");
    // �������������� ��������� �����
    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "listen failed with error: " << WSAGetLastError() << "\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    int count = 0;
    printf("5.accept \n");
    // ��������� �������� ����������
    while (true)
    {
        count++;
        printf("Connection number: %d\n",count);
        int client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            cerr << "accept failed: " << WSAGetLastError() << "\n";
            closesocket(listen_socket);
            WSACleanup();
            return 1;
        }

        const int max_client_buffer_size = 1024;
        char buf[max_client_buffer_size];

        printf("6.recv \n");
        result = recv(client_socket, buf, max_client_buffer_size, 0);

        std::stringstream response; // ���� ����� ������������ ����� �������
        std::stringstream response_body; // ���� ������

        if (result == SOCKET_ERROR) {
            // ������ ��������� ������
            cerr << "recv failed: " << result << "\n";
            closesocket(client_socket);
        }
        else if (result == 0) {
            // ���������� ������� ��������
            cerr << "connection closed...\n";
        }
        else if (result > 0) {

            // �� ����� ����������� ������ ���������� ������, ������� ������ ����� ����� ������
            // � ������ �������.
            buf[result] = '\0';

            printf("%d\n", result);
            printf("%s\n", buf);

            // ������ ������� ��������
            // ��������� ���� ������ (HTML)
            //response_body << "<title>Test C++ HTTP Server</title>\n"
            //    << "<h1>Test page</h1>\n"
            //    << "<p>This is body of the test page...</p>\n"
            //    << "<h2>Request headers</h2>\n"
            //    << "<pre>" << buf << "</pre>\n"
            //    << "<em><small>Test C++ Http Server</small></em>\n";

            response_body << "<link rel=icon href=data:;base64,=>"
                << "<title>spectrtaldev.ru</title>"
                << "<h1 style=color:green;text-align:center;margin-top:5em;>spectraldev.ru</h1>";

            // ��������� ���� ����� ������ � �����������
            response << "HTTP/1.1 200 OK\r\n"
                << "Version: HTTP/1.1\r\n"
                << "Content-Type: text/html; charset=utf-8\r\n"
                << "Content-Length: " << response_body.str().length()
                << "\r\n\r\n"
                << response_body.str();

            // ���������� ����� ������� � ������� ������� send
            result = send(client_socket, response.str().c_str(),
                response.str().length(), 0);

            if (result == SOCKET_ERROR) {
                // ��������� ������ ��� �������� ������
                cerr << "send failed: " << WSAGetLastError() << "\n";
            }
            // ��������� ���������� � ��������
            closesocket(client_socket);
        }
    }
    // ������� �� �����
    closesocket(listen_socket);
    freeaddrinfo(addr);
    WSACleanup();
    return 0;
}