#include<vector>
#include<functional>
#include<iostream>

using namespace std;
int main() {
	std::vector<std::function<void(void)>>commandlist;
	commandlist.push_back([]() {
		cout << "GPU Set RTV-" << endl; });//����1
	cout << "CPU Set����2" << endl;

}