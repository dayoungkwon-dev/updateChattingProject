#pragma comment(lib, "ws2_32.lib") //������� ���̺귯���� ��ũ. ���� ���̺귯�� ����

#include <WinSock2.h>
#include <string>
#include <iostream>
#include <thread>
#include <vector>
#include <mysql/jdbc.h>
#include <sstream>


#define MAX_SIZE 1024
#define MAX_CLIENT 10

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::vector;
using std::istringstream;


struct SOCKET_INFO { // ����� ���� ������ ���� Ʋ ����
    SOCKET sck;
    string user;
    int ti;
};

std::vector<SOCKET_INFO> sck_list; // ����� Ŭ���̾�Ʈ ���ϵ��� ������ �迭 ����.
SOCKET_INFO server_sock; // ���� ���Ͽ� ���� ������ ������ ���� ����.
int client_count = 0; // ���� ������ �ִ� Ŭ���̾�Ʈ�� count �� ���� ����.
std::vector<string> pctList = {};

void server_init(); // socket �ʱ�ȭ �Լ�. socket(), bind(), listen() �Լ� �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
void add_client(int ti); // ���Ͽ� ������ �õ��ϴ� client�� �߰�(accept)�ϴ� �Լ�. client accept() �Լ� �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
void send_msg(const char* msg); // send() �Լ� �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
void recv_msg(string user); // recv() �Լ� �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
void del_client(int idx); // ���Ͽ� ����Ǿ� �ִ� client�� �����ϴ� �Լ�. closesocket() �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
SOCKET getSocket(string user);
int removeSocket(string user);
int recreate = -1;
std::thread th1[MAX_CLIENT];
void insertPtcpt();
void insertMsgInfo(string msg);


// DB ����
void connectionDB();
string selectQuery(string user, string funcName, string sql);
string testQ(sql::ResultSet* res, string user);
string insertQuery(string user, string sql);
string updateQuery(string user, string sql);


const string server = "tcp://127.0.0.1:3306"; // �����ͺ��̽� �ּ�
const string username = "root"; // �����ͺ��̽� �����
const string password = "1122"; // �����ͺ��̽� ���� ��й�ȣ

sql::mysql::MySQL_Driver* driver;
sql::Connection* con;


void recreateThread() {
    while (1) {
        //cout << "recreate " << recreate << endl;
        if (recreate > -1) {
            //cout << "th1.join() " << recreate << endl;
            th1[recreate].join();
            //cout << "join " << recreate << endl;
            th1[recreate] = std::thread(add_client, recreate);
            recreate = -1;
        }
        if (recreate == -2) {
            return;
        }
        Sleep(1000);
    }
}


int main() {
    WSADATA wsa;

    // Winsock�� �ʱ�ȭ�ϴ� �Լ�. MAKEWORD(2, 2)�� Winsock�� 2.2 ������ ����ϰڴٴ� �ǹ�.
    // ���࿡ �����ϸ� 0��, �����ϸ� �� �̿��� ���� ��ȯ.
    // 0�� ��ȯ�ߴٴ� ���� Winsock�� ����� �غ� �Ǿ��ٴ� �ǹ�.
    int code = WSAStartup(MAKEWORD(2, 2), &wsa);

    if (!code) {
        server_init();
        connectionDB();         //DB����

        for (int i = 0; i < MAX_CLIENT; i++) {
            // �ο� �� ��ŭ thread �����ؼ� ������ Ŭ���̾�Ʈ�� ���ÿ� ������ �� �ֵ��� ��.
            th1[i] = std::thread(add_client, i);
        }
        //std::thread th1(add_client); // �̷��� �ϸ� �ϳ��� client�� �޾���...
        std::thread th2(recreateThread);

        while (1) { // ���� �ݺ����� ����Ͽ� ������ ����ؼ� ä�� ���� �� �ִ� ���¸� ����� ��. �ݺ����� ������� ������ �� ���� ���� �� ����.
            string text, msg = "";

            std::getline(cin, text);
            const char* buf = text.c_str();
            msg = server_sock.user + " : " + buf;
            send_msg(msg.c_str());
        }

        for (int i = 0; i < MAX_CLIENT; i++) {
            th1[i].join();
            //join : �ش��ϴ� thread ���� ������ �����ϸ� �����ϴ� �Լ�.
            //join �Լ��� ������ main �Լ��� ���� ����Ǿ thread�� �Ҹ��ϰ� ��.
            //thread �۾��� ���� ������ main �Լ��� ������ �ʵ��� ����.
        }
        //th1.join();
        th2.join();

        closesocket(server_sock.sck);
    }
    else {
        cout << "���α׷� ����. (Error code : " << code << ")";
    }

    WSACleanup();

    return 0;
}


void server_init() {
    server_sock.sck = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Internet�� Stream ������� ���� ����
   // SOCKET_INFO�� ���� ��ü�� socket �Լ� ��ȯ��(��ũ���� ����)
   // ���ͳ� �ּ�ü��, ��������, TCP �������� �� ��. 

    SOCKADDR_IN server_addr = {}; // ���� �ּ� ���� ����
    // ���ͳ� ���� �ּ�ü�� server_addr

    server_addr.sin_family = AF_INET; // ������ Internet Ÿ�� 
    server_addr.sin_port = htons(7777); // ���� ��Ʈ ����
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // �����̱� ������ local �����Ѵ�. 
    //Any�� ���� ȣ��Ʈ�� 127.0.0.1�� ��Ƶ� �ǰ� localhost�� ��Ƶ� �ǰ� ���� �� ����ϰ� �� �� �ֵ�. �װ��� INADDR_ANY�̴�.
    //ip �ּҸ� ������ �� server_addr.sin_addr.s_addr -- ������ ���?

    int b = bind(server_sock.sck, (sockaddr*)&server_addr, sizeof(server_addr)); // ������ ���� ������ ���Ͽ� ���ε��Ѵ�.
    int l = listen(server_sock.sck, SOMAXCONN); // ������ ��� ���·� ��ٸ���.
    //cout << "b " << b << ", l " << l << endl;
    server_sock.user = "server";

    cout << "Server On" << endl;
    insertPtcpt();
}


// ���⼭ �� �Դ� ���� �Ѵ�
void add_client(int ti) {
    SOCKADDR_IN addr = {};
    int addrsize = sizeof(addr);
    char buf[MAX_SIZE] = { };

    ZeroMemory(&addr, addrsize); // addr�� �޸� ������ 0���� �ʱ�ȭ

    SOCKET_INFO new_client = {};
    //cout << "before accept" << endl;

    new_client.sck = accept(server_sock.sck, (sockaddr*)&addr, &addrsize);
    //cout << "after accept" << endl;
    recv(new_client.sck, buf, MAX_SIZE, 0);
    // Winsock2�� recv �Լ�. client�� ���� �г����� ����.
    new_client.user = string(buf);
    new_client.ti = ti;
    


    string query, substr; // temp�����ϴ� substr ����
    query = new_client.user;
    istringstream ss(query);
    vector<string> stream;

    while (getline(ss, substr, ' ')) {
        stream.push_back(substr); // [0] : ` ,[1] : id, [2] : �Լ�, [3] sql���� 
    }

    

    if (stream[0] == "`") {
        string finalRes;
        string user = stream[1];
        string funcName = stream[2];
        string select = stream[3];
        std::thread th(recv_msg, user);
        int eraseLength = 0;
        eraseLength = size(stream[0]) + size(stream[1]) + size(stream[2]) + 3;
        query.erase(0, eraseLength); // ������ �� ���ܳ��� ��

        cout << "buf :::: " << new_client.user << endl;

        if (select == "SELECT")
        {
            string msg= "����";
            // cout << "sck :: " << new_client.sck << endl;
            
            finalRes = selectQuery(user, funcName, query);
            send(new_client.sck, finalRes.c_str(), MAX_SIZE, 0);
        }
        else if (select == "INSERT")
        {
            insertQuery(user, query);
        }
        else if (select == "UPDATE")
        {
            updateQuery(user, query);
        }
        
    }
    else {
        string msg = "[����] " + new_client.user + " ���� �����߽��ϴ�.";

        pctList.push_back(new_client.user);
        insertPtcpt();

        cout << msg << endl;
        sck_list.push_back(new_client); // client ������ ��� sck_list �迭�� ���ο� client �߰�

        std::thread th(recv_msg, new_client.user);
        // �ٸ� �����κ��� ���� �޽����� ����ؼ� ���� �� �ִ� ���·� ����� �α�.

        client_count++; // client �� ����.
        cout << "[����] ���� ������ �� : " << client_count << "��" << endl;
        send(new_client.sck, msg.c_str(), MAX_SIZE, 0);
        send_msg(msg.c_str()); // c_str : string Ÿ���� const chqr* Ÿ������ �ٲ���.

        th.join();
        //cout << "th.join()" << endl;
    }
}


void send_msg(const char* msg) {
    for (int i = 0; i < sck_list.size(); i++) { // ������ �ִ� ��� client���� �޽��� ����
        send(sck_list[i].sck, msg, MAX_SIZE, 0);
    }

    // �޼��� insert DB
    insertMsgInfo(msg);
}


void recv_msg(string user) {
    char buf[MAX_SIZE] = { };
    string msg = "";
    SOCKET sck = getSocket(user);
    int pIdx = 0;
    // user�� ���ϸ���Ʈ���� �� ��°���� �Ǵ��ϴ� userIdx��� int ���� �ϳ� �����
    int userIdx;

    //cout << sck_list[idx].user << endl;

    while (1) {
        ZeroMemory(&buf, MAX_SIZE);
        //cout << "recv" << endl;
        int x = 0;
        x = recv(sck, buf, MAX_SIZE, 0);
        msg = user + " : " + buf;

        std::stringstream ss(msg);  // ���ڿ��� ��Ʈ��ȭ
        string stream1, stream2, stream3;
        // ��Ʈ���� ����, ���ڿ��� ���� �и��� ������ �Ҵ�.
        ss >> stream1; // ù ��° �ܾ�
        ss >> stream2; // �� ��° �ܾ�
        ss >> stream3; // �� ��° �ܾ�


        if (stream3 == "/s")
        {
            // ���⼭ ���ϸ���Ʈ���� �̸��� user�� ����� �� ��° �ε������� ã�ƾ� ��
            for (int i = 0; i < sck_list.size(); i++)
            {
                if (sck_list[i].user == user)
                {
                    userIdx = i;
                }
            }
            // ���ϸ���Ʈ���� �̸��� user�� ����� userIdx ��°���
            msg = "������ 1��1�� ������ �޼��� �׽�Ʈ";
            send(sck_list[userIdx].sck, msg.c_str(), MAX_SIZE, 0); // �ش� user���� �� ���� 
        }
        else if (msg == user + " : /q" || x < 1)
        {
            msg = "[����] " + user + " ���� �����߽��ϴ�.";

            // ������ DB ����.
            pIdx = std::find(pctList.begin(), pctList.end(), user) - pctList.begin();
            pctList.erase(pctList.begin() + pIdx);
            insertPtcpt();

            cout << msg << endl;
            

            send_msg(msg.c_str()); // �̰� ��ü���� ������ ����


            //del_client(idx); // Ŭ���̾�Ʈ ����
            int remove = removeSocket(user);
            //cout << "remove " << remove << endl;
            if (remove > -1) {
                recreate = remove;
                //cout << "set recreate " << recreate << endl;
            }
            return;
        }
        else
        {
            cout << msg << endl;
            send_msg(msg.c_str());
        }
    }
    //cout << "recv_msg out" << endl;
}


void del_client(int idx) {
    closesocket(sck_list[idx].sck);
    sck_list.erase(sck_list.begin() + idx); // �迭���� Ŭ���̾�Ʈ�� �����ϰ� �� ��� index�� �޶����鼭 ��Ÿ�� ���� �߻�....��
    client_count--;
}


SOCKET getSocket(string user) {
    for (int i = 0; i < sck_list.size(); i++) { // ������ �ִ� ��� client���� �޽��� ����
        if (sck_list[i].user == user) {
            return sck_list[i].sck;
        }
    }
    return 0;
}


int removeSocket(string user) {
    int ti;
    for (int i = 0; i < sck_list.size(); i++) { // ������ �ִ� ��� client���� �޽��� ����
        if (sck_list[i].user == user) {
            ti = sck_list[i].ti;
            del_client(i);
            return ti;
        }
    }
    return -1;
}


// ������ DB insert
void insertPtcpt()
{
    // MySQL Connector/C++ �ʱ�ȭ
    sql::mysql::MySQL_Driver* driver; // ���� �������� �ʾƵ� Connector/C++�� �ڵ����� ������ ��
    sql::Connection* con;

    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(server, username, password);
    }
    catch (sql::SQLException& e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        exit(1);
    }

    // �����ͺ��̽� ����
    con->setSchema("chattingproject");

    // ������ ��� ���� (�ߺ�����)
    sql::PreparedStatement* delPstmt;
    delPstmt = con->prepareStatement("TRUNCATE TABLE participant");
    delPstmt->execute();
    delete delPstmt;

    // ������ insert
    for (int i = 0; i < pctList.size(); i++) {
        sql::PreparedStatement* pstmt;
        pstmt = con->prepareStatement("INSERT INTO participant(memberID) VALUES(?)");
        pstmt->setString(1, pctList[i]);
        pstmt->execute();
        delete pstmt;
    }

    delete con;
}


void insertMsgInfo(string msg)
{
    std::stringstream ss(msg);  // ���ڿ��� ��Ʈ��ȭ
    string stream1 = "", stream2 = "", stream3 = "", stream4 = "";
    string dmYN = "0";
    // 
    // 1: �۽���, 3 : /D , 4 : ������, �ð��� ����ð����� �ֱ�.
    ss >> stream1; // ù ��° �ܾ�
    ss >> stream2; // �� ��° �ܾ�
    ss >> stream3; // �� ��° �ܾ�
    ss >> stream4; // �� ��° �ܾ�

    // ��ɾ�� '/D','/S'�� ����. 
    if (stream3 != "/d" && stream3 != "/s" && stream3 != "/f" && stream3 != "/F") {
        if (stream3 == "/D") {
            dmYN = "1";
        }
        else {
            stream4 = "";
        }

        // ������ memberID null.
        if (stream1 == "[����]") {
            stream1 = "";
        }

        // MySQL Connector/C++ �ʱ�ȭ
        sql::mysql::MySQL_Driver* driver; // ���� �������� �ʾƵ� Connector/C++�� �ڵ����� ������ ��
        sql::Connection* con;
        sql::PreparedStatement* pstmt;
        sql::Statement* stmt;

        try {
            driver = sql::mysql::get_mysql_driver_instance();
            con = driver->connect(server, username, password);
        }
        catch (sql::SQLException& e) {
            cout << "Could not connect to server. Error message: " << e.what() << endl;
            exit(1);
        }

        // �����ͺ��̽� ����
        con->setSchema("chattingproject");

        // db �ѱ� ������ ���� ���� 
        stmt = con->createStatement();
        stmt->execute("set names euckr");
        if (stmt) { delete stmt; stmt = nullptr; }

        // ������ insert
        pstmt = con->prepareStatement("INSERT INTO chat(memberID, chatContent, chatDateTime, DM, receiverID) VALUES(?,?,date_format(now(), '%Y-%m-%d %H:%m:%s'), ?,?);");
        pstmt->setString(1, stream1);
        pstmt->setString(2, msg);
        pstmt->setString(3, dmYN);
        pstmt->setString(4, stream4);
        pstmt->execute();

        delete pstmt;
        delete con;
    }
}


// DB ����
void connectionDB() {
    
    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(server, username, password);
    }
    catch (sql::SQLException& e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        exit(1);
    }

    // �����ͺ��̽� ����
    con->setSchema("chattingproject");
}


// SELECT �Լ�
string selectQuery(string user, string funcName, string sql)
{
    string result;
    sql::Statement* stmt;
    sql::ResultSet* res;

    cout << "sql :: " << sql << endl;

    // select�� ����
    stmt = con->createStatement();
    res = stmt->executeQuery(sql);

    cout << "selectQuery :: " << user << endl;



    if (funcName == "testFunc") {
        result = testQ(res, user);
    }

    delete stmt;
    delete res;

    return result;
}


// func�� �Լ�(test)
string testQ(sql::ResultSet* res, string user)
{
    string result, id = "";
    int i = 1;

    cout << "testQ :: �Լ� ����" << endl;
    cout << "user :: " << user << endl;

    while (res->next()) {
        cout << i << ". ";
        cout << "ID : " << res->getString("memberID") << ",";
        cout << "PW : " << res->getString("passWord") << endl;
        result += i + ". ID : " + res->getString("memberID") + ", PW : " + res->getString("passWord") + "\n";
        i++;
    }

    /*
    string idYN = "false";
    while (res->next()) {
        id = res->getString("memberID");
        if (id != "") {
            idYN = "true";
        }
    }
    result = idYN;
    */

    return result;
}


// INSERT �Լ�
string insertQuery(string user, string sql)
{
    string successYN = "";
    sql::Statement* stmt;
    sql::ResultSet* res;

    // insert�� ����
    stmt = con->createStatement();
    res = stmt->executeQuery(sql);

    delete stmt;
    delete res;

    return successYN;
}


// UPDATE �Լ�
string updateQuery(string user, string sql)
{
    string successYN = "";
    sql::Statement* stmt;
    sql::ResultSet* res;

    // db �ѱ� ������ ���� ���� 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    // update�� ����
    stmt = con->createStatement();
    res = stmt->executeQuery(sql);

    delete stmt;
    delete res;

    return successYN;
}