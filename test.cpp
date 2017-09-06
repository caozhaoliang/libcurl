#include <iostream>

using namespace std;

class test
{
public:
	test(){}
	// test(int n):m_(n){}
	~test(){}
private:
	int m_;	
};

int main()
{
	test* t = new test[10];

	return 0;

}