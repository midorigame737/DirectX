#include<vector>
#include<functional>
#include<iostream>

using namespace std;
int main() {
	std::vector<std::function<void(void)>>commandlist;
	commandlist.push_back([]() {
		cout << "GPU Set RTV-" << endl; });//–½—ß1
	cout << "CPU Set–½—ß2" << endl;

}