#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;
using std::vector;

int main(int argc, char** argv) {

    //lpos와 rpos를 저장할 pair
    pair<int, int> temp;
    //frame별로 pair 저장
    vector<pair<int, int>> line;

    ofstream outfile;

    //lpos rpos 값 입력
    for (int i = 0; i < 100; i++)
    {
        temp = make_pair(i, i);
        line.push_back(temp);
    }

    //lpos rpos값을 저장할 csv파일 open
    outfile.open("test.csv", ios::out);
    
    //csv파일에 저장
    for (int j = 0; j < line.size(); j++)
    {
        outfile << line[j].first <<","<< line[j].second << endl;
    }
    outfile.close();

    // 콘솔에서 csv에 입력된 값 출력하기(확인용)
    //string str;
    //ifstream input("test.csv");
    //while (!input.eof()) {
    //    getline(input, str);
    //    cout << str << endl;
    //}
    //input.close();

    return 0;
}