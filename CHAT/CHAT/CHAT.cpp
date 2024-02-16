#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)
#include <vector>
#include <map>

#include <winsock2.h>   
#include <thread>

using namespace std;

void set_config_addr(SOCKADDR_IN& addr) {
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //ip
    addr.sin_family = AF_INET; //семейство для интернет протокола
    addr.sin_port = htons(1111); //порт
}

vector<SOCKET> Connections;
map<string, SOCKET> Con;
map<string, vector<SOCKET>> Conn;

map<string, vector<SOCKET>> groups;
int c = 0;

void make_group(string name_of_chat, int n) {
    char name[20];
    Conn[name_of_chat].push_back(Connections[n]);
    while (string(name) != "exit"){
        recv(Connections[n], name, 20, NULL);
        string nn = string(name);
        if(nn != "exit")
            Conn[name_of_chat].push_back(Con[nn]);
    }
}

void clientHandler(int n) {
    
    while (true) {
        int msg_size;
        char name[20];
        char whom_to_send[20];
        char name_of_chat[20];
        char time[5];

        recv(Connections[n], whom_to_send, 20, NULL);
        string g = string(whom_to_send);
        if (g.find("chat") == std::string::npos) {
            recv(Connections[n], name, 20, NULL);
            recv(Connections[n], (char*)&msg_size, sizeof(int), NULL);
            char* msg = new char[msg_size + 1];
            msg[msg_size] = '\0';
            recv(Connections[n], msg, msg_size, NULL);
            recv(Connections[n], time, 5, NULL);
            if (msg_size != 0) {
                for (int i = 0; i < Conn[whom_to_send].size(); i++) {
                    if (Conn[whom_to_send][i] != Connections[n]) {
                        if (Conn[whom_to_send].size() > 1)
                            send(Conn[whom_to_send][i], whom_to_send, 20, NULL);
                        else {
                            char is_c[20];
                            for (int i = 0; i < 20; i++)
                            {
                                is_c[i] = '.';
                            }
                            send(Conn[whom_to_send][i], is_c, 20, NULL);
                        }
                        send(Conn[whom_to_send][i], name, 20, NULL);
                        send(Conn[whom_to_send][i], (char*)&msg_size, sizeof(int), NULL);
                        send(Conn[whom_to_send][i], msg, msg_size, NULL);
                        send(Conn[whom_to_send][i], time, 5, NULL);
                    }

                }
            }
            delete[] msg;
        }
        else if (string(whom_to_send) == "createchat") {
            char name_of_chat[20];
            recv(Connections[n], name_of_chat, 20, NULL);
            make_group(string(name_of_chat), n);
        }
        /*else{
            recv(Connections[n], name_of_chat, 20, NULL);
            recv(Connections[n], name, 20, NULL);

            recv(Connections[n], (char*)&msg_size, sizeof(int), NULL);
            char* msg = new char[msg_size + 1];
            msg[msg_size] = '\0';
            recv(Connections[n], msg, msg_size, NULL);
            recv(Connections[n], time, 5, NULL);
            for (int i = 0; i < groups[name_of_chat].size(); i++) {
                if (groups[name_of_chat][i] != Connections[n]) {
                    send(groups[name_of_chat][i], name, 20, NULL);
                    send(groups[name_of_chat][i], (char*)&msg_size, sizeof(int), NULL);
                    send(groups[name_of_chat][i], msg, msg_size, NULL);
                    send(groups[name_of_chat][i], time, 5, NULL);
                }
            }
        }*/
        
    }
}


int main()
{
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        cout << "Не удалось установить библиотеку";
        exit(1);
    } //устанавливаем библиотеку

    SOCKADDR_IN addr; //инфа о адресе сокета
    int size_n = sizeof(addr);
    set_config_addr(addr);

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL); //сокет для прослушивания
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); //привязывает адрес сокета для прослушки
    listen(sListen, SOMAXCONN);//запускает прослушивание

    SOCKET newConnection; //сокет для удержания соединения с клиентом
    for (int i = 0; i < 100; i++)
    {
        newConnection = accept(sListen, (SOCKADDR*)&addr, &size_n);  //после выполнения addr будет содержать сведения о клиенте, который
        //выполнил подключение

        if (newConnection == 0) {
            cout << "Error #2";
        }
        else {
            char nn[256];

            recv(newConnection, nn, 20, NULL);
            Con[nn] = newConnection;
            Conn[nn].push_back(newConnection);
            cout << "Client connected\n";
            char msg[256];
            Connections.push_back(newConnection);

            CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)clientHandler, (LPVOID)(i), NULL, NULL);
        }
    }
    system("pause");

}
