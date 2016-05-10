#include "stdafx.h"
#include "CommandHandler.h"

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		std::cout << "Usage: commands.json input.json output.txt" << std::endl;
		return -1;
	}

	std::string f(argv[1]);
	std::ifstream in(argv[1]);
	CStateMachineProcessor smProcesor(argv[2], argv[3]);
	CCommandHandler handler(smProcesor, in);

	while (!in.eof() && !in.fail())
	{
		handler.HandleCommand();
	}

    return 0;
}

