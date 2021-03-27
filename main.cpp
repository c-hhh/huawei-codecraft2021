//
// Created by chhhh on 2021/3/27.
//








#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include <math.h>
using namespace std;

const string FILENAME = "D://file/training-1.txt";

struct Server
{
    Server() {}
    Server(string name, int cores, int mem, int h_cost, int d_cost) :
            server_name(name), cpu_cores(cores), memory(mem), hardware_cost(h_cost), day_cost(d_cost) {}
    /* data */
    string  server_name;
    int     cpu_cores;
    int     memory;
    int     hardware_cost;
    int     day_cost;
};

struct ProcuredServer
{
    ProcuredServer() {}
    ProcuredServer(string name, int A_r_cores, int B_r_cores, int A_r_mem, int B_r_mem) :
            server_name(name), A_remain_cores(A_r_cores), B_remain_cores(B_r_cores), A_remain_memory(A_r_mem), B_remain_memory(B_r_mem) {
        serverID = -1;
    }
    /* data */
    string  server_name;
    int     serverID;
    int     A_remain_cores;
    int     B_remain_cores;
    int     A_remain_memory;
    int     B_remain_memory;
    double  A_cm_specific;  // A结点核心内存之比
    double  B_cm_specific;  // B结点核心内存之比
    bool    changeRemain(string method, char node, int c, int m) {
        if (method == "add") {
            if (node == 'A' && A_remain_cores - c >= 0 && A_remain_memory - m >= 0) {
                A_remain_cores -= c;
                A_remain_memory -= m;
                A_cm_specific = (double)A_remain_cores / (double)A_remain_memory;
                return true;
            } else if (node == 'B' && B_remain_cores - c >= 0 && B_remain_memory - m >= 0) {
                B_remain_cores -= c;
                B_remain_memory -= m;
                B_cm_specific = (double)B_remain_cores / (double)B_remain_memory;
                return true;
            }
        } else if (method == "del") {
            if (node == 'A') {
                A_remain_cores += c;
                A_remain_memory += m;
                A_cm_specific = (double)A_remain_cores / (double)A_remain_memory;
                return true;
            } else if (node == 'B') {
                B_remain_cores += c;
                B_remain_memory += m;
                B_cm_specific = (double)B_remain_cores / (double)B_remain_memory;
                return true;
            }
        }
    }
};

struct VM
{
    VM() {}
    VM(string name, int cores, int mem, bool is_d) :
            vm_name(name), vm_cores(cores), vm_memory(mem), is_double_node(is_d) {}
    /* data */
    string  vm_name;
    int     vm_cores;
    int     vm_memory;
    bool    is_double_node;
};

struct UserVM
{
    UserVM() {}
    UserVM(uint32_t id, string vm, string sn, int sid, bool idn, int np = 0) :
            user_id(id), vm_name(vm), server_name(sn), server_id(sid), is_double_node(idn), node_pos(np) {}
    /* data */
    uint32_t user_id;
    string  vm_name;
    string  server_name;
    int     server_id;
    bool    is_double_node;
    int     node_pos;   // 如果是单节点部署，则有效；否则默认双节点；0A 1B
};

struct UserReq
{
    UserReq() {}
    UserReq(int op, string vm_n, uint32_t u_id) :
            operate(op), vm_name(vm_n), user_id(u_id) {}
    UserReq(int op, uint32_t u_id) :
            operate(op), user_id(u_id) {
        vm_name = "";
    }
    /* data */
    int operate;    // add is 1, del is 0;
    uint32_t user_id;
    string   vm_name;
};

int vminc = INT32_MAX;
int vminm = INT32_MAX;

unordered_map<string, Server>       servers;    // 服务器种类
unordered_map<string, VM>           vms;        // 虚拟机种类
unordered_map<uint32_t, UserVM>     userVMs;    // 用户创建的虚拟机
vector<ProcuredServer>          boughtServers;  // 已购买的服务器
/* 每天请求的临时变量存在这里 */
list<UserReq> userReq;    // add和del按顺序存在了里面,详情看结构体的定义
unordered_map<string, int> newServers;  // 每日新购的服务器
vector<pair<int, int>> newVMs;
vector<string> newVMsInfo;
int lastDayServersNum;

long long money;
long long eachday;
void cal() {
    money += eachday;
}


vector<string> split(const string& sstr);
void readServers();
void readVM();
int  readDays();
int  readReqCount();
void readReq(int days);
void addNewServer(string s_id);
bool creatVM(UserReq& oneReq);
bool addDoubleNode(UserReq& oneReq, VM& vm);
bool addSingleNode(UserReq& oneReq, VM& vm);
bool canPutIn(ProcuredServer& bs, VM& vm, bool is_double, int node);  // 当is_double为假,node表示判断是A还是B
void delVM(UserReq& oneReq);
pair<string, vector<int>> buy(int day); // -1->A; 0->double; 1->B
void creatServerID();
void newVMsInfoToStr();
int addVMReq(vector<int>& info);

void readServersF(ifstream& inFile);
void readVMF(ifstream& inFile);
int  readDaysF(ifstream& inFile);
int  readReqCountF(ifstream& inFile);
void readReqF(int days, ifstream& inFile);

int aaa;

int main() {
    lastDayServersNum = 0;
    ifstream inFile(FILENAME);
    money = 0; eachday = 0; aaa = 0;
    ///readServers();
    ///readVM();
    ///int T = readDays();

    readServersF(inFile);
    readVMF(inFile);
    int T = readDaysF(inFile);

    while (T--) {
        ///int R = readDays();   // 读取每天要处理的请求数量

        int R = readDaysF(inFile);
        // int curServersNum = boughtServers.size();
        userReq.clear();
        newServers.clear();
        newVMs.clear();
        newVMsInfo.clear();
        ///readReq(R);
        readReqF(R, inFile);
        /*  决策部分,需要得到
            购买服务器种类数     (purchase, Q)
            每种服务器购买方案   (型号名, 数量)
            虚拟机迁移数量       (migration, W)
            W行的迁移数据        (虚拟机 ID, 目的服务器 ID)
                                (虚拟机 ID, 目的服务器 ID, 目的服务器节点)
            新创建虚拟机的部署    (服务器 ID) 或 (服务器 ID, 部署节点)
        */
        int number = 0;
        while (R--) {
            auto oneReq = userReq.front();
            if (oneReq.operate == 1) {  // add
                if (!creatVM(oneReq)) {
                    auto info = buy(T + 1);
                    int pos = info.second[0];
                    if(pos == -2){
                        R++;
                        continue;
                    }
                    else{
                        string buyServerName = info.first;
                        addNewServer(buyServerName);
                        int reqNum = addVMReq(info.second);
                        R++;
                        R -= reqNum;
                        continue;
                    }
                }
                number++;
            }
            else {
                delVM(oneReq);
            }
        }
        /* 输出： */
        creatServerID();
        /*cout << "(purchase, " << newServers.size() << ")" << endl;
        aaa += newServers.size();

        for (auto& newServer : newServers) {
            cout << "(" << newServer.first << ", " << newServer.second << ")" << endl;
        }
        cout << "(migration, 0)" << endl;
        // cout << "迁移"<<endl;
        newVMsInfoToStr();
        for (int i = 0; i < newVMsInfo.size(); i++) {
            cout << newVMsInfo[i] << endl;
        }*/

        cal();
    }

    cout << boughtServers.size() << endl;
    cout << money << endl;
    return 0;
}

/* 给该请求分配服务器空间 */
void loggingVMInfo(int id, bool is_double, int node) {
    pair<int, int> tmp_pair;    // <存储的服务器的位置编号，存的节点位置>
    tmp_pair.first = id;
    if (is_double) {
        tmp_pair.second = -1;
    }
    else {
        tmp_pair.second = node;
    }
    newVMs.push_back(tmp_pair);
}

int addVMReq(vector<int>& info) {
    int retReqNum = 0;  // 处理的请求数量
    ProcuredServer aimServer = boughtServers[boughtServers.size() - 1]; // 放入的目标服务器
    int aimSerIdx = boughtServers.size() - 1;
    for (int num = 0; num < info.size(); num++) {
        auto oneReq = userReq.front();
        // 若当前操作为删除操作，执行del()
        if (oneReq.operate == 0) {
            delVM(oneReq);
            retReqNum++;
            num--;
            continue;
        }
        /* add请求处理 */

        VM vm = vms[oneReq.vm_name];
        bool is_double = vm.is_double_node;
        int pos = info[num] == -1 ? 0 : 1;
        userVMs[oneReq.user_id] = UserVM(oneReq.user_id, oneReq.vm_name, aimServer.server_name, boughtServers.size() - 1, is_double, pos);
        if (is_double) {
            boughtServers[aimSerIdx].changeRemain("add", 'A', vm.vm_cores/2, vm.vm_memory/2);
            boughtServers[aimSerIdx].changeRemain("add", 'B', vm.vm_cores/2, vm.vm_memory/2);
        }
        else if (pos == 0) {
            boughtServers[aimSerIdx].changeRemain("add", 'A', vm.vm_cores, vm.vm_memory);
        }
        else {
            boughtServers[aimSerIdx].changeRemain("add", 'B', vm.vm_cores, vm.vm_memory);
        }
        loggingVMInfo(aimSerIdx, is_double, pos);   // 存入vm在server配置的信息
        userReq.pop_front();
        retReqNum++;
    }
    return retReqNum;
}

void newVMsInfoToStr() {
    for (auto& vm : newVMs) {
        int s_id = boughtServers[vm.first].serverID;
        string str = "(" + to_string(s_id);
        if (vm.second == -1) {
            str += ")";
        }
        else if (vm.second == 0) {
            str += ", A)";
        }
        else if (vm.second == 1) {
            str += ", B)";
        }
        newVMsInfo.push_back(str);
    }
}

void creatServerID() {
    int num = 0;
    for (auto& ns : newServers) {
        num = ns.second;
        for (int i = boughtServers.size() - 1; i >= 0; i--) {
            // 从尾部向前找对应名字的服务器
            while (boughtServers[i].server_name != ns.first)    i--;
            boughtServers[i].serverID = lastDayServersNum;
            lastDayServersNum++; num--;
            if (num == 0) {
                break;
            }
        }
    }
}

/* 添加服务器 */
void addNewServer(string s_id) {
    Server tmp = servers[s_id];
    ProcuredServer server(s_id, tmp.cpu_cores / 2, tmp.cpu_cores / 2, tmp.memory / 2, tmp.memory / 2);
    boughtServers.push_back(server);
    if (newServers.find(server.server_name) == newServers.end()) {
        newServers[server.server_name] = 1;
    }
    else {
        newServers[server.server_name]++;
    }
    eachday += servers[server.server_name].day_cost;
    money += servers[server.server_name].hardware_cost;
}

/* 删除请求 */
void delVM(UserReq& oneReq) {
    if (oneReq.operate == 1) { cout << "错误的请求分配：add请求在del中执行！" << endl; exit; }

    int sid = userVMs[oneReq.user_id].server_id;
    bool is_db = userVMs[oneReq.user_id].is_double_node;
    VM  vm = vms[userVMs[oneReq.user_id].vm_name];
    int delCoresNum = vm.vm_cores,
            delMemNum = vm.vm_memory;
    if (is_db) {    // 如果是双节点部署
        boughtServers[sid].changeRemain("del", 'A', delCoresNum/2, delMemNum/2);
        boughtServers[sid].changeRemain("del", 'B', delCoresNum/2, delMemNum/2);
    }
    else {        // 如果是单节点部署
        int node = userVMs[oneReq.user_id].node_pos;
        if (node == 0) {// 从A删除
            boughtServers[sid].changeRemain("del", 'A', delCoresNum, delMemNum);
        }
        else {          // 从B删除
            boughtServers[sid].changeRemain("del", 'B', delCoresNum, delMemNum);
        }
    }
    auto it = userVMs.find(oneReq.user_id);
    if (it != userVMs.end())
    {
        userVMs.erase(it);
        userReq.pop_front();
    }
    else {
        cout << "删除用户虚拟机发生删除异常" << endl; exit;
    }
    return;
}

bool creatVM(UserReq& oneReq) {
    VM vm = vms[oneReq.vm_name];
    bool b = vm.is_double_node;
    if (b)  return addDoubleNode(oneReq, vm);
    else    return addSingleNode(oneReq, vm);
}
bool addDoubleNode(UserReq& oneReq, VM& vm) {
    int cores = vm.vm_cores,
            memory = vm.vm_memory;
    vector<int> temp;
    temp.clear();
    bool flag = false;
    for (int i = 0; i < boughtServers.size(); i++) {
        if (canPutIn(boughtServers[i], vm, true, 0)) {
            flag = true;
            temp.push_back(i);
        }
    }
    if(flag){
        int pos = temp[0];
        int c = min(boughtServers[temp[0]].A_remain_cores, boughtServers[temp[0]].B_remain_cores);
        int m = min(boughtServers[temp[0]].A_remain_memory, boughtServers[temp[0]].B_remain_memory);
        for(int i = 1; i < temp.size(); i++){
            int cp = min(boughtServers[temp[i]].A_remain_cores, boughtServers[temp[i]].B_remain_cores);
            int mp = min(boughtServers[temp[i]].A_remain_memory, boughtServers[temp[i]].B_remain_memory);
            if(mp + cp < m + c){
                pos = temp[i];
                c = cp;
                m = mp;
            }
        }
        boughtServers[pos].changeRemain("add", 'A', vm.vm_cores/2, vm.vm_memory/2);
        boughtServers[pos].changeRemain("add", 'B', vm.vm_cores/2, vm.vm_memory/2);
        userVMs[oneReq.user_id] = UserVM(oneReq.user_id, oneReq.vm_name, boughtServers[pos].server_name, pos, true);
        loggingVMInfo(pos, true, 0);
        userReq.pop_front();
    }
    return flag;
}

bool cmp(const pair<int, int>& a, const pair<int, int>& b){
    double cmpA, cmpB;
    if (a.second == 0)
        cmpA = boughtServers[a.first].A_cm_specific;
    else    cmpA = boughtServers[a.first].B_cm_specific;
    if (b.second == 0)
        cmpB = boughtServers[b.first].A_cm_specific;
    else    cmpB = boughtServers[b.first].B_cm_specific;
    return cmpA < cmpB;
}
bool addSingleNode(UserReq& oneReq, VM& vm) {
    int cores = vm.vm_cores,
            memory = vm.vm_memory;
    vector<pair<int, int>> temp;    // 可放入的服务器<serverIdx, node>
    for (int i = 0; i < boughtServers.size(); i++) {
        if (canPutIn(boughtServers[i], vm, false, 0)) {
            temp.push_back(make_pair(i, 0));
        }
        if (canPutIn(boughtServers[i], vm, false, 1)) {
            temp.push_back(make_pair(i, 1));
        }
    }
    // 如果没有合适的位置，返回false
    if (temp.size() == 0)   return false;
    sort(temp.begin(), temp.end(), cmp);

    double _bizhi = (double)vm.vm_cores / (double)vm.vm_memory;
    double cha = 10000;
    int idx;
    for (int i = 0; i < temp.size(); i++) {
        double _cha = cha;
        if (temp[i].second == 0)
            cha = min(cha, fabs(_bizhi - boughtServers[temp[i].first].A_cm_specific));
        else cha= min(cha, fabs(_bizhi - boughtServers[temp[i].first].B_cm_specific));
        if (cha != _cha)    idx = i;
    }
    auto aimNode = temp[idx];
    if (aimNode.second == 0)
        boughtServers[aimNode.first].changeRemain("add", 'A', vm.vm_cores, vm.vm_memory);
    else boughtServers[aimNode.first].changeRemain("add", 'B',vm.vm_cores, vm.vm_memory);
    userVMs[oneReq.user_id] = UserVM(oneReq.user_id, oneReq.vm_name, boughtServers[aimNode.first].server_name, aimNode.first, false, aimNode.second);
    loggingVMInfo(aimNode.first, false, aimNode.second);
    userReq.pop_front();
    return true;
}
bool isFit(int remainCores, int remainMem) {///////////////////////////
    remainCores++; remainMem++;
    // cout << remainCores << " " << remainMem << endl;
    // 判定函数
    if ((remainCores < 5 && remainMem > 45) || (remainCores > 45 && remainMem < 5))  return false;
    if ((remainCores < 10 && remainMem > 150) || (remainCores > 150 && remainMem < 10))  return false;
    ///if ((remainCores < vminc + 10 && remainMem > vminm + 50) || (remainCores > vminc + 150 && remainMem < vminm + 15))  return false;
    return true;
}
bool canPutIn(ProcuredServer& bs, VM& vm, bool is_double, int node) {
    int cEachNode = vm.vm_cores / (is_double ? 2 : 1),
            mEachNode = vm.vm_memory / (is_double ? 2 : 1);
    int A_rc = bs.A_remain_cores, A_rm = bs.A_remain_memory,
            B_rc = bs.B_remain_cores, B_rm = bs.B_remain_memory;
    // 如果是双节点部署
    if (is_double) {
        if (A_rc >= cEachNode && B_rc >= cEachNode && A_rm >= mEachNode && B_rm >= mEachNode) {
            if (isFit(A_rc - cEachNode, A_rm - mEachNode)
                && isFit(B_rc - cEachNode, B_rm - mEachNode)) {
                return true;
            }
        }
    }
    else {  // 如果是单节点部署
        if (node == 0) { // 假定部署在A节点
            if (A_rc >= cEachNode && A_rm >= mEachNode && isFit(A_rc - cEachNode, A_rm - mEachNode)) {
                return true;
            }
        }
        else {        // 假定部署在B节点
            if (B_rc >= cEachNode && B_rm >= mEachNode && isFit(B_rc - cEachNode, B_rm - mEachNode)) {
                return true;
            }
        }
    }
    return false;
}


/* 读取服务器数据 */
void readServers() {
    string _N;
    getline(cin, _N);
    int N = stoi(_N);
    //cin >> N;
    string line;
    while (N--) {
        getline(cin, line);
        vector<string>&& tmp = split(line);
        servers[tmp[0]] = Server(tmp[0], stoi(tmp[1]), stoi(tmp[2]), stoi(tmp[3]), stoi(tmp[4]));
    }
}

/* 读取虚拟机种类 */
void readVM() {
    string _M;
    getline(cin, _M);
    int M = stoi(_M);
    string line;
    while (M--) {
        getline(cin, line);
        vector<string>&& tmp = split(line);
        vms[tmp[0]] = VM(tmp[0], stoi(tmp[1]), stoi(tmp[2]), bool(stoi(tmp[3])));
        int num = 1;

        if(bool(stoi(tmp[3])))
            num = 2;

        vminm = min(vms[tmp[0]].vm_memory / num, vminm);
        vminc = min(vms[tmp[0]].vm_cores / num, vminc);
    }
}

/* 读取天数 */
int readDays() {
    string _T;
    getline(cin, _T);
    int T = stoi(_T);
    return T;
}

/* 读取每天的请求数量 */
int readReqCount() {
    string _R;
    getline(cin, _R);
    int R = stoi(_R);
    return R;
}

/* 读取一天的请求 */
void readReq(int days) {
    string line;
    while (days--) {
        getline(cin, line);
        vector<string>&& tmp = split(line);
        if (tmp[0] == "add") {
            userReq.push_back(UserReq(1, tmp[1], uint32_t(stoll(tmp[2]))));
        }
        else if (tmp[0] == "del") {
            userReq.push_back(UserReq(0, uint32_t(stoll(tmp[1]))));
        }
    }
}

/* 将单行数据以字符串数组的形式返回 */
vector<string> split(const string& sstr) {
    string str = sstr.substr(1, sstr.size() - 2);   //not sure
    vector<string> ret;
    int idx = 1;
    do {
        if (str[idx] == ',')    str[idx] = ' ';
    } while (++idx != str.size() - 1);
    string data;
    istringstream tmp(str);
    while (tmp >> data)
        ret.push_back(data);
    return ret;
}

string _buy() {
    int n = servers.size();
    int r_n = rand() % (0 - n);
    for (auto& server : servers) {
        if (r_n == 0) {
            return server.first;
        }
        r_n--;
    }
    return "";
}


/* 购买的服务器 */
pair<string, vector<int>> buy(int day) {
    pair<string, vector<int>> ans;
    int lc = 0;
    int lm = 0;
    int rc = 0;
    int rm = 0;
    vector<int> pos;
    vector<string> ser;
    ser.clear();
    vector<string> temp;
    int tempPos;
    vector<UserReq> tempReq;
    for (list<UserReq>::iterator it = userReq.begin(); it != userReq.end(); ++it) {
        if (it->operate == 0)
            continue;
        tempReq.push_back(*it);
        int c = vms[it->vm_name].vm_cores;
        int m = vms[it->vm_name].vm_memory;
        bool t = vms[it->vm_name].is_double_node;
        if (t) {
            tempPos = 0;
            lc += (c / 2);
            rc += (c / 2);
            lm += (m / 2);
            rm += (m / 2);
        }
        else if (lc <= rc && lm <= rm) {
            tempPos = -1;
            lc += c;
            lm += m;
        }
        else  if(lc > rc && lm > rm){
            tempPos = 1;
            rc += c;
            rm += m;
        }
        else{
            break;
        }
        for (unordered_map<string, Server>::iterator it = servers.begin(); it != servers.end(); it++) {
            if (it->second.cpu_cores / 2 < max(lc, rc) || it->second.memory / 2 < max(lm, rm)
                ||!isFit(it->second.cpu_cores / 2 - lc, it->second.memory / 2 - lm)
                ||!isFit(it->second.cpu_cores / 2 - rc, it->second.memory / 2 - rm))
                continue;
            temp.push_back(it->second.server_name);
        }
        if (temp.size() == 0)
            break;
        ser = temp;
        temp.clear();
        pos.push_back(tempPos);
    }
    if (pos.size() <= 3) {
        string ansSer;
        if (ser.size() == 0) {
            string srtname = _buy();
            ans.first = srtname;
            pos.push_back(1);
            pos[0] = -2;
            ans.second = pos;
            return ans;
        }
        for (int minCost = INT32_MAX, i = 0; i < ser.size(); i++) {
            if (minCost > servers[ser[i]].hardware_cost + servers[ser[i]].day_cost * day) {
                minCost = servers[ser[i]].hardware_cost + servers[ser[i]].day_cost * day;
                ansSer = ser[i];
            }
        }
        ans.first = ansSer;
        ans.second = pos;
        return ans;
    }
    else{//////////////////////
        string ansSer;
        int n = pos.size();
        vector<int> pos1;
        vector<int> pos2;
        string s1;
        string s2;
        int minc = INT32_MAX;
        int minc1;
        int minc2;
        string mins;
        vector<int> minp;
        vector<string> temp1;
        vector<string> temp2;
        for(int i = 2; i <= n / 2; i++){
            int lc1 = 0;
            int lm1 = 0;
            int rc1 = 0;
            int rm1 = 0;
            int lc2 = 0;
            int lm2 = 0;
            int rc2 = 0;
            int rm2 = 0;
            for(int j = 0; j < i; j++) {
                int c = vms[tempReq[j].vm_name].vm_cores;
                int m = vms[tempReq[j].vm_name].vm_memory;
                bool t = vms[tempReq[j].vm_name].is_double_node;
                if (t) {
                    tempPos = 0;
                    lc1 += (c / 2);
                    rc1 += (c / 2);
                    lm1 += (m / 2);
                    rm1 += (m / 2);
                }
                else if (lc <= rc && lm <= rm) {
                    tempPos = -1;
                    lc1 += c;
                    lm1 += m;
                }
                else{
                    tempPos = 1;
                    rc1 += c;
                    rm1 += m;
                }
                pos1.push_back(tempPos);
            }
            for (unordered_map<string, Server>::iterator it = servers.begin(); it != servers.end(); it++) {
                if (it->second.cpu_cores / 2 < max(lc1, rc1) || it->second.memory / 2 < max(lm1, rm1)
                    ||!isFit(it->second.cpu_cores / 2 - lc1, it->second.memory / 2 - lm1)
                    ||!isFit(it->second.cpu_cores / 2 - rc1, it->second.memory / 2 - rm1))
                    continue;
                temp1.push_back(it->second.server_name);
            }
            for (int minc1 = INT32_MAX, i = 0; i < temp1.size(); i++) {
                if (minc1 > servers[temp1[i]].hardware_cost + servers[temp1[i]].day_cost * day) {
                    minc1 = servers[temp1[i]].hardware_cost + servers[temp1[i]].day_cost * day;
                    s1 = temp1[i];
                }
            }
            for(int j = i; j < n; j++){
                int c = vms[tempReq[j].vm_name].vm_cores;
                int m = vms[tempReq[j].vm_name].vm_memory;
                bool t = vms[tempReq[j].vm_name].is_double_node;
                if (t) {
                    tempPos = 0;
                    lc2 += (c / 2);
                    rc2 += (c / 2);
                    lm2 += (m / 2);
                    rm2 += (m / 2);
                }
                else if (lc <= rc && lm <= rm) {
                    tempPos = -1;
                    lc2 += c;
                    lm2 += m;
                }
                else{
                    tempPos = 1;
                    rc2 += c;
                    rm2 += m;
                }
                pos2.push_back(tempPos);
            }
            for (unordered_map<string, Server>::iterator it = servers.begin(); it != servers.end(); it++) {
                if (it->second.cpu_cores / 2 < max(lc1, rc1) || it->second.memory / 2 < max(lm1, rm1)
                    ||!isFit(it->second.cpu_cores / 2 - lc1, it->second.memory / 2 - lm1)
                    ||!isFit(it->second.cpu_cores / 2 - rc1, it->second.memory / 2 - rm1))
                    continue;
                temp2.push_back(it->second.server_name);
            }
            for (int minc2 = INT32_MAX, i = 0; i < temp2.size(); i++) {
                if (minc1 > servers[temp2[i]].hardware_cost + servers[temp2[i]].day_cost * day) {
                    minc1 = servers[temp2[i]].hardware_cost + servers[temp2[i]].day_cost * day;
                    s2 = temp2[i];
                }
            }
            if(minc > minc1 + minc2) {
                minc = minc1 + minc2;
                mins = s1;
                minp = pos1;
            }
        }
        ans.first = mins;
        ans.second = minp;
        return ans;
    }
}



/* 读取服务器数据 */
void readServersF(ifstream& inFile) {
    int N;
    string s_serverKinds;
    getline(inFile, s_serverKinds);
    istringstream tmp(s_serverKinds);
    string line;
    tmp >> N;
    while (N--) {
        getline(inFile, line);
        vector<string>&& tmp = split(line);
        servers[tmp[0]] = Server(tmp[0], stoi(tmp[1]), stoi(tmp[2]), stoi(tmp[3]), stoi(tmp[4]));
    }
}

/* 读取虚拟机种类 */
void readVMF(ifstream& inFile) {
    int M;
    string s_VMKinds;
    getline(inFile, s_VMKinds);
    istringstream tmp(s_VMKinds);
    string line;
    tmp >> M;
    while (M--) {
        getline(inFile, line);
        vector<string>&& tmp = split(line);
        vms[tmp[0]] = VM(tmp[0], stoi(tmp[1]), stoi(tmp[2]), bool(stoi(tmp[3])));
    }
}

/* 读取天数 */
int readDaysF(ifstream& inFile) {
    int T;
    string s;
    getline(inFile, s);
    istringstream tmp(s);
    tmp >> T;
    return T;
}

/*  */
int readReqCountF(ifstream& inFile) {
    int R;
    string s;
    getline(inFile, s);
    istringstream tmp(s);
    tmp >> R;
    return R;
}

/* 读取一天的请求 */
void readReqF(int days, ifstream& inFile) {
    string line;
    while (days--) {
        getline(inFile, line);
        vector<string>&& tmp = split(line);
        if (tmp[0] == "add") {
            userReq.push_back(UserReq(1, tmp[1], uint32_t(stoll(tmp[2]))));
        }
        else if (tmp[0] == "del") {
            userReq.push_back(UserReq(0, uint32_t(stoll(tmp[1]))));
        }
    }
}

