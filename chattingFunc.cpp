#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>

using namespace std;

enum color {
    BLACK = 0,
    DARK_BLUE,
    DARK_RED = 4,
    GRAY = 7,
    BLUE = 9,
    GREEN,
    SKYBLUE,
    RED,
    YELLOW = 14,
    WHITE
};

void textcolor(int foreground, int background);
vector<vector<string>> getPtcpt(string myId);
vector<string> useSpeaker(string myId);

// DM ��� �Էº�
string inputDM(string myId)
{
    vector<vector<string>> pList;
    int end = 0;
    string newFrdNum, message;
    pList = getPtcpt(myId);
    while (end == 0)
    {
        std::cout << "DM �� ��� id �Է� : ";
        cin >> newFrdNum;
        vector<string> newFriend;
        for (int i = 1; i < pList.size() + 1; i++)
        {
            newFriend = pList[i - 1];
            if (newFrdNum == newFriend[1])
            {
                if (newFriend[2] == "N")
                {
                    cout << "ģ���� �ƴ� �����ڿ��Դ� DM �� �� �����ϴ�." << endl;
                    end = 1;
                    return "";
                    break;
                }
                else if (newFriend[2] == "Y")
                {
                    cout << "���� �޼��� �Է� : ";
                    getline(cin, message);
                    getline(cin, message);
                    return  "/D " + newFriend[1] + " " + message; // �۽��� : /D ������ �޼���
                    end = 1;
                    break;
                }
            }
        }
        if (end == 0)
        {
            cout << "\n            * �Է� ���� *" << endl;
            cout << "### ������ ����� id�� �Է��ϼ���. ###" << endl;
        }
    }
}


// DM ��� ��º�
void outputDM(string stream1, string stream2, string stream3, string stream4, string msg, string myId)
{
    if (stream3 == "/D" && stream4 == myId)
    {
        int eraseLength = 0;
        eraseLength = size(stream1) + size(stream2) + size(stream3) + size(stream4) + 3;
        msg.erase(0, eraseLength);
        cout << "                                                            ";
        textcolor(BLACK, YELLOW); 
        cout << stream1 << "�� �ӼӸ�";
        textcolor(GRAY, BLACK);
        cout << " :" << msg << endl;
    }
}


// ģ���߰� ��� �Էº�
string inputFriend(string myId)
{
    vector<vector<string>> pList;
    int end = 0;
    string newFrdNum;
    pList = getPtcpt(myId);
    while (end == 0)
    {
        cout << "ģ����û �� ��� id �Է� : ";
        cin >> newFrdNum;
        string temp;
        getline(cin, temp);
        vector<string> newFriend;
        for (int i = 1; i < pList.size() + 1; i++)
        {
            newFriend = pList[i - 1];
            if (newFrdNum == newFriend[1])
            {
                if (newFriend[2] == "N")
                {
                    return "/F " + newFriend[1]; // �۽��� : /F ������
                    end = 1;
                    break;
                }
                else if (newFriend[2] == "Y")
                {
                    cout << "�̹� ģ���Դϴ�." << endl;
                    end = 1;
                    return "";
                    break;
                }
            }
        }
        if (end == 0)
        {
            cout << "\n            * �Է� ���� *" << endl;
            cout << "### ������ ����� id�� �Է��ϼ���. ###" << endl;
        }
    }
}


// ģ���߰� ��� ��º�
tuple<string, string, int> outputFriend(string stream1, string stream3, string stream4, string myId)
{
    if (stream3 == "/F" && stream4 == myId)
    {
        cout << "                                                            " << "ID '" << stream1 << "'��(��) ģ�� ��û�� ���½��ϴ�. �����Ͻðڽ��ϱ�?(Y, N)\n :";
        return { stream1 , stream4, 1 };
    }
    return { "" , "", 0 };
}


// Ȯ���� ��� �Էº�
void inputSpeaker(string myId, SOCKET client_sock)
{
    vector<string> groupInfo;
    string groupName, message, text;
    int gSize = groupInfo.size();
    groupInfo = useSpeaker(myId);

    groupName = groupInfo[0];
    cout << "[ ";
    if (groupName == "red")
    {
        textcolor(RED, BLACK);
    }
    else if (groupName == "green")
    {
        textcolor(GREEN, BLACK);
    }
    else if (groupName == "blue")
    {
        textcolor(BLUE, BLACK);
    }
    else if (groupName == "yellow")
    {
        textcolor(YELLOW, BLACK);
    }
    cout << groupName;
    textcolor(GRAY, BLACK);
    cout << " ] ������ ���� �޼��� �Է� : ";
    getline(cin, message);

    for (int i = 1; i < groupInfo.size(); i++)
    {
        text = "/T " + groupName + " " + groupInfo.at(i) + " " + message; // �۽��� : /T �׷��̸� ������ �޼���
        const char* buffer = text.c_str(); // string���� char* Ÿ������ ��ȯ
        send(client_sock, buffer, strlen(buffer), 0); // ������
    }
}


// Ȯ���� ��� ��º� (�۽��� : /T �׷��̸� ������ �޼���)
void outputSpeaker(string stream1, string stream2, string stream3, string stream4, string stream5, string msg, string myId)
{
    if (stream3 == "/T" && stream5 == myId)
    {
        int eraseLength = 0;
        eraseLength = size(stream1) + size(stream2) + size(stream3) + size(stream4) + size(stream5) + 4;
        msg.erase(0, eraseLength);
        if (stream4 == "red")
        {
            textcolor(RED, BLACK);
        }
        else if (stream4 == "green")
        {
            textcolor(GREEN, BLACK);
        }
        else if (stream4 == "blue")
        {
            textcolor(BLUE, BLACK);
        }
        else if (stream4 == "yellow")
        {
            textcolor(YELLOW, BLACK);
        }
        cout << "                                                            " << stream1 << "�� �׷� �޼��� :" << msg << endl;
        textcolor(GRAY, BLACK);
    }
}