#include <iostream>
#include <mysql/jdbc.h>
#include <string>
#include <sstream>
#include <vector>


using namespace std;

const string server = "tcp://127.0.0.1:3306"; // �����ͺ��̽� �ּ�
const string username = "root"; // �����ͺ��̽� �����
const string password = "1122"; // �����ͺ��̽� ���� ��й�ȣ



// ������ ��� ��� (��ü)
vector<vector<string>> getPtcpt(string myId) {
    int i = 0;
    string no, friendYN = "N";
    vector<vector<string>> pList;
    vector<string> p1;
    vector<string> fList;

    // MySQL Connector/C++ �ʱ�ȭ
    sql::mysql::MySQL_Driver* driver;
    sql::Connection* con;
    sql::Statement* pStmt;
    sql::Statement* fStmt;
    sql::ResultSet* pRes;
    sql::ResultSet* fRes;

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

    // ������ ��� select.
    pStmt = con->createStatement();
    pRes = pStmt->executeQuery("SELECT memberID FROM participant WHERE NOT memberID ='" + myId + "'");
    delete pStmt;


    // ������ ID pList�� �ֱ�.
    while (pRes->next()) {
        no = to_string(i + 1);
        p1.push_back(no);                           // ������ ��ȣ
        p1.push_back(pRes->getString("memberID"));  // ������ ID

        pList.push_back(p1);
        i++;
        p1.clear();
    }


    // ģ�� ��� select.
    fStmt = con->createStatement();
    string sql = "SELECT friendList FROM member WHERE memberID ='" + myId + "'";
    fRes = fStmt->executeQuery(sql);
    delete fStmt;

    // ģ����� ������ ����.
    while (fRes->next()) {
        istringstream ss1(fRes->getString("friendList"));
        string buffer;
        while (getline(ss1, buffer, ',')) {
            fList.push_back(buffer);
        }
    }

    // ģ������ �Ǻ��Ͽ� friendYN �� �ֱ�.
    for (int i = 0; i < pList.size(); i++) {
        for (int j = 0; j < fList.size(); j++) {
            if (pList[i][1] == fList[j]) {
                friendYN = "Y";
            }
        }
        pList.at(i).push_back(friendYN);   // �����ڿ� �α���ID�� ģ������(Y/N)
        friendYN = "N";
    }
    delete fRes;
    delete pRes;
    delete con;


    // ������ ��� ���.
    cout << "\n �� ������ ��� ��" << endl;
    for (int j = 0; j < pList.size(); j++) {
        cout << pList.at(j).at(0) << ". ";
        cout << pList.at(j).at(1);
        cout << " - ģ������ : " << pList.at(j).at(2) << endl;
    }
    return pList;
}


// �� ä�� (���� �׷쳻 ID ��ȸ)
vector<string> useSpeaker(string myId) {
    // MySQL Connector/C++ �ʱ�ȭ
    sql::mysql::MySQL_Driver* driver;
    sql::Connection* con;
    sql::Statement* stmt;
    sql::ResultSet* res;
    vector<string> groupInfo;
    string groupName;

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

    // ���� �׷� ID ��ȸ
    stmt = con->createStatement();
    res = stmt->executeQuery("SELECT memberID, groupName FROM member WHERE groupName = (SELECT groupName FROM member WHERE memberID ='" + myId + "')");
    delete stmt;

    while (res->next()) {
        groupName = (res->getString("groupName"));
        if (groupInfo.empty()) {
            groupInfo.push_back(groupName);
        }
        groupInfo.push_back(res->getString("memberID"));
    }

    return groupInfo;
}


// �α��� �ߺ�üũ (������ �� ��ȸ)
string loginCheck(string myId) {
    int count = 0;
    string loginYN = "N";
    string selectId = "";

    // MySQL Connector/C++ �ʱ�ȭ
    sql::mysql::MySQL_Driver* driver;
    sql::Connection* con;
    sql::Statement* pStmt;
    sql::ResultSet* pRes;
    sql::Statement* fStmt;
    sql::ResultSet* fRes;


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

    // ������ ��� select.
    pStmt = con->createStatement();
    pRes = pStmt->executeQuery("SELECT memberID FROM participant WHERE memberID ='" + myId + "'");
    delete pStmt;

    // ������ �� ���� ID ���翩�� Ȯ��.
    while (pRes->next()) {
        selectId = pRes->getString("memberID");
        if (selectId == myId) {
            loginYN = "Y";
        }
    }


    // ������ ���� 10�� �������� ���� Ȯ��  
    fStmt = con->createStatement();
    fRes = fStmt->executeQuery("SELECT count(*) AS cnt FROM participant");
    delete fStmt;
    while (fRes->next()) {
        count = fRes->getInt("cnt");
    }
    if (count >= 10)
    {
        loginYN = "F";
    }

    return loginYN;
}