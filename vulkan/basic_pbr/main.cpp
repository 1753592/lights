#include "../VulkanView.h"

class VulkanViewPbr : public VulkanView {

public:
	void createDrawables() override
	{
		createScene1();
	}
};


int main(int argc, char** argv)
{
	try {
		VulkanViewPbr vv;
		vv.run();
	}
	catch (std::runtime_error& e) {
		printf("%s", e.what());
		return -1;
	}
	return 0;
}