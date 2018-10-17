#include <iostream>
#include "Application.hpp"

int main(int argc, char** argv)
{
	try {
		nanogui::init();

		{
			nanogui::ref<Application> app = new Application();
			app->drawAll();
			app->setVisible(true);
			nanogui::mainloop();
		}

		nanogui::shutdown();
	}
	catch (const std::runtime_error &e) {
		std::string error = std::string("Caught a fatal error: ") + std::string(e.what());
		std::cerr << error << std::endl;
		return -1;
	}
	
	return 0;
}