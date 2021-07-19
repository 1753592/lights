#include "../VulkanView.h"



int main(int argc, char** argv)
{
	try {
		VulkanView vv;
		vv.run();
	}
	catch (std::runtime_error& e) {
		printf("%s", e.what());
		return -1;
	}
	return 0;
}