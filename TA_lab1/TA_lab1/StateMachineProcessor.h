#pragma once
#include "StateMachine.h"

typedef std::map<std::string, std::pair<std::string, std::string>> States;

class CStateMachineProcessor
{
public:
	CStateMachineProcessor() = delete;
	CStateMachineProcessor(std::string const& input, std::string const& output);
	void WriteToFile();
	void TransferToMeale(CStateMachine & sm);
	void TransferToMoore(CStateMachine & sm);
	void Determine(CStateMachine & sm);
	void Minimize(CStateMachine & sm);
	CStateMachine & Get(std::string const& id);
	~CStateMachineProcessor();
private:
	StateTable AllocateOfEquivalenceClass(StateTable resourceST, StateTable const &originalST);
	json_spirit::Object ToJson(CStateMachine const& sm);
	json_spirit::Array GetStates(CStateMachine const& sm);
	json_spirit::Array GetTransitions(CStateMachine const& sm);
	States GetNewStates(StateTable const &meale);
	std::vector<CStateMachine> m_stateMachines;
	json_spirit::Array m_jsonStateMachines;
	bool m_isFirstTimeWrite;
	std::string m_output;

};

