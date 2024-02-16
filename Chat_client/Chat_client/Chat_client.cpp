#include <iostream>
#include <thread>
#include <string>
#include <ctime>
#include <vector>
#include <time.h>
#include <map>
#include <chrono>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>   
#pragma warning(disable:4996)


using namespace std;


SOCKET Connection;
char n[20];
char other_n[20];
char whom_to_send[20];
char is_chat[20];

string a;
string currentDateTime() {
    std::time_t t = std::time(0); // get time now
    std::tm* now = std::localtime(&t);
    int hour = now->tm_hour;
    int minutes = now->tm_min;
    pair<int, int> p;
    p.first = hour; p.second = minutes;
    string a;
    if(minutes > 10)
    a = to_string(p.first) + ":" + to_string(p.second);
    else 
        a = to_string(p.first) + ":" + "0" + to_string(p.second);

    return a;
}

class Message {//класс описывающий сообщения
public:
    string mess; //текст сообщения
    string from_who; //от кого
    string date; //дата получения
    bool new_or;
    Message() {
        new_or = true;
    }
};
map<string, vector<Message>> messages; //все сообщения клиенту от других клиентов

void set_config_addr(SOCKADDR_IN& addr) {
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //ip
    addr.sin_family = AF_INET; //семейство для интернет протокола
    addr.sin_port = htons(1111); //порт
}
bool f = false;
void send_to_server() {//функция для отправки сокетов на сервер
    char t[5];
    while (true) {
        string s;
        if (!f) {
            cout << "Your chats: \n";
            for (auto i: messages) {
                int count_of_new_messages = 0; 
                for (int j = 0; j < messages[i.first].size(); j++) {
                    if (messages[i.first][j].new_or == true and n != messages[i.first][j].from_who and messages[i.first][j].mess != "")
                        count_of_new_messages++;
                }
                cout << i.first << "     " << count_of_new_messages << "\n";
            }
            cout << "\n";
            cout << "What chat: ";
            cin >> a; //вводим то, в какой чат заходим

            if (string(a).find("chat") == std::string::npos) {
                for (int i = 0;i < messages[a].size(); i++) {//выводим все сообщения с этим чатом
                    cout << messages[a][i].from_who << ": " << messages[a][i].mess << "     ";
                    for (int d = 0; d < 5; d++)
                    {
                        cout << messages[a][i].date[d];
                    }
                    cout << endl;
                    messages[a][i].new_or = false;
                }
                f = true;
            }
            else if (string(a) == "createchat") {
                const char* cstr = a.c_str();
                send(Connection, cstr, 20, NULL);
                char name_of_chat[20];
                cout << "Enter name of chat: ";
                cin >> name_of_chat;
                send(Connection, name_of_chat, 20, NULL);
                char name[20];
                while (string(name) != "exit") {
                    cin >> name;
                    send(Connection, name, 20, NULL);
                    
                }
                cout << "Chat created! \n";
            }
        }
        if(f){
            getline(cin, s);// вводим сообщение
            string time = currentDateTime();
            for (int i = 0; i < time.size(); i++)
            {
                t[i] = time[i];
            }
            if (s == "exit") {
                f = false;
                cout << "\n";
            }

            else {
                Message mess;
                mess.mess = s;
                mess.from_who = string(n);
                mess.date = time;

                messages[a].push_back(mess);

                int size_m = s.size();
                const char* cstr = a.c_str();
                send(Connection, cstr, 20, NULL);//отправляем имя кому отправляем
                send(Connection, n, 20, NULL);//отправляем на сервер наше имя
                send(Connection, (char*)&size_m, sizeof(int), NULL);//отправляем размер сообщения
                send(Connection, s.c_str(), size_m, NULL);//отправляем сообщение
                send(Connection, t, 5, NULL);//отправляем на сервер время сообщения
                Sleep(10);
            }
        }
    }
}
void get_from_Server() { //функция для получения сокетов от сервера
    int msg_size;
    char time[5];
    while (true) {
        recv(Connection, is_chat, 20, NULL);//chat или нет, если "...", то нет
        recv(Connection, other_n, 20, NULL);//получаем имя отправителя
        recv(Connection, (char*)&msg_size, sizeof(int), NULL);//получаем размер сообщения
        char* msg = new char[msg_size + 1];
        msg[msg_size] = '\0';
        recv(Connection, msg, msg_size, NULL);//получаем само сообщение
        recv(Connection, time, 5, NULL);//получаем время отправления сообщения

        Message mess;
        mess.mess = string(msg);
        mess.from_who = string(other_n);
        mess.date = string(time);
        if (string(is_chat) == "...................." and msg_size!=0)
        messages[other_n].push_back(mess);
        else if(msg_size != 0)
            messages[is_chat].push_back(mess);
        
        if (f) { //выводит сообщение, которое мы получаем во время нахождения в чате
            if (string(a) == string(is_chat) or string(a) == string(other_n)) {
                if (string(a) == string(is_chat)) messages[is_chat][messages[is_chat].size() - 1].new_or = false;
                if (string(a) == string(other_n)) messages[other_n][messages[other_n].size() - 1].new_or = false;

                cout << mess.from_who << ": " << mess.mess << "     ";
                for (int i = 0; i < 5; i++)
                {
                    cout << time[i];
                }cout << endl;
            }
        }
        delete[] msg;

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

    Connection = socket(AF_INET, SOCK_STREAM, NULL);
    cout << "Enter name: ";
    cin >> n;
    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) { //устанавливаем соединение
        cout << "Error! Failed connection to server";
        return 1;
    }
    cout << n << " connected\n";
    send(Connection, n, 20, NULL);
    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)get_from_Server, (LPVOID)(Connection), NULL, NULL);
    send_to_server();

    system("pause");
    return 0;
}