#include<vector>
#include<functional>
#include<iostream>

using namespace std;
int main() {
	std::vector<std::function<void(void)>>commandlist;//�^���I�ȃR�}���h���X�g
	commandlist.push_back([]() {
		cout << "GPU SetRTV-1" << endl;
		});//����1
	cout << "CPU Set����2" << endl;
	commandlist.push_back([]() {
		cout << "GPU Clear RTV-3" << endl;
		});//����2
}