#include <iostream>
#include "Application.hpp"
#include "CommandLine.hpp"

int main(int argc, char** argv)
{
	try {
		
		if (argc == 2 || argc == 3) {
			std::unique_ptr<Lykta::CommandLine> cmd = std::unique_ptr<Lykta::CommandLine>(new Lykta::CommandLine(argc, argv));
		}
		else {
			nanogui::init();

			{
				nanogui::ref<Lykta::Application> app = new Lykta::Application();
				app->drawAll();
				app->setVisible(true);
				nanogui::mainloop();
			}

			nanogui::shutdown();
		}
	}
	catch (const std::runtime_error &e) {
		std::string error = std::string("Caught a fatal error: ") + std::string(e.what());
		std::cerr << error << std::endl;
		return -1;
	}
	
	return 0;
}