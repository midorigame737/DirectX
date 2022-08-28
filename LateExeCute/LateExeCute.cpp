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
	cout << "CPU Clear����-4" << endl;
	commandlist.push_back([]() {
		cout << "GPUClose-5" << endl;
		});
	cout << "GPU Close ����-6" << endl;
	//�R�}���h�L���[��ExecuteCommand��͂�������
	for (auto cmd : commandlist) {
		cmd();
	}
	getchar();
	return 0;
}