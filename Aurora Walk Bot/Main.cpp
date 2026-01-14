#include <iostream>
#include "reader.hpp"
#include "walkbot.h"
using namespace std;


void offsetUpdateLoop() {
	while (true) {
		g_game.loop();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}




int main()
{
	if (!mem::find_driver()) {
		system("color 2");
		cout << "\n Driver not found!\n";
	}
	mem::process_id = mem::find_process("cs2.exe");
	
	virtualaddy = mem::find_image();
	std::cout << virtualaddy << std::endl;
	g_game.init();

	std::thread updater(offsetUpdateLoop);
	WalkBot bot;
	std::this_thread::sleep_for(std::chrono::milliseconds(2050));
	while (true) {

		
		bot.Run();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	cin.get();

}