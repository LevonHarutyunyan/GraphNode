#include "GraphNode.h"

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cout << "Usage: -port" << std::endl;
		return 1;
	}

	GraphNode node(std::atoi(argv[1]));
	node.run_connection_process();

}
