#include "stdafx.h"
#include "StateMachineProcessor.h"


int main()
{
	auto stateMachinesProc = CStateMachineProcessor("input.json");
	stateMachinesProc.TransferToMoore(stateMachinesProc.Get("state machine x"));
	stateMachinesProc.TransferToMeale(stateMachinesProc.Get("state machine y"));
	stateMachinesProc.WriteToFile("output.json");
    return 0;
}

