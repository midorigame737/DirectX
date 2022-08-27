#include<vector>
#include<functional>
#include<iostream>

using namespace std;
int main() {
	std::vector<std::function<void(void)>>commandlist;//疑似的なコマンドリスト
	commandlist.push_back([]() {
		cout << "GPU SetRTV-1" << endl;
		});//命令1
	cout << "CPU Set命令2" << endl;
	commandlist.push_back([]() {
		cout << "GPU Clear RTV-3" << endl;
		});//命令2
}