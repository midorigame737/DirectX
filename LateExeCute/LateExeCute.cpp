#include<vector>
#include<functional>
#include<iostream>

using namespace std;
int main() {
	std::vector<std::function<void(void)>>commandlist;//^IÈR}hXg
	commandlist.push_back([]() {
		cout << "GPU SetRTV-1" << endl;
		});//½ß1
	cout << "CPU Set½ß2" << endl;
	commandlist.push_back([]() {
		cout << "GPU Clear RTV-3" << endl;
		});//½ß2
}