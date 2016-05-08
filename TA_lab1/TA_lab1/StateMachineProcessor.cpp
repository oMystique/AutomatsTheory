#include "stdafx.h"
#include "StateMachineProcessor.h"

using namespace json_spirit;
using namespace std;

CStateMachineProcessor::CStateMachineProcessor(std::string const& input)
	: m_isFirstTimeWrite(true)
{
	ifstream inputFile(input);
	Value value;
	read(inputFile, value);
	auto stateMachinesArray = value.get_obj().at(0).value_.get_array();
	for (size_t i = 0; i < stateMachinesArray.size(); i++)
	{
		m_stateMachines.push_back(CStateMachine(stateMachinesArray.at(i).get_obj()));
	}
}

void CStateMachineProcessor::WriteToFile(std::string const& output)
{
	ofstream out(output);
	Object mainObj;
	mainObj.push_back(Pair("state machine", m_jsonStateMachines));
	write(mainObj, out, json_spirit::pretty_print);
}

json_spirit::Object CStateMachineProcessor::ToJson(CStateMachine const& sm)
{
	Object stateMachine;
	stateMachine.push_back(Pair("id", sm.GetId()));
	stateMachine.push_back(Pair("type", sm.GetType()));
	stateMachine.push_back(Pair("states", GetStates(sm)));
	stateMachine.push_back(Pair("transitions", GetTransitions(sm)));
	return stateMachine;
}

json_spirit::Array CStateMachineProcessor::GetStates(CStateMachine const& sm)
{
	Array states;
	auto smStates = sm.GetTable()[0];
	for (size_t i = 1; i < smStates.size(); i++)
	{
		Object state = {Pair("id", smStates[i].first)};
		if (smStates[i].second != "")
		{
			state.push_back(Pair("output", smStates[i].second));
		}
		states.push_back(state);
	}
	return states;
}

json_spirit::Array CStateMachineProcessor::GetTransitions(CStateMachine const& sm)
{
	Array transitions;
	auto states = sm.GetTable()[0];
	for (size_t row = 1; row < sm.GetTable().size(); row++)
	{
		for (size_t col = 1; col < states.size(); col++)
		{
			if (sm.GetTable()[row][col].first != "")
			{
				Object cell;
				cell.push_back(Pair("input", sm.GetTable()[row][0].first));
				if (states[col].second == "")
				{
					cell.push_back(Pair("output", sm.GetTable()[row][col].second));
				}
				cell.push_back(Pair("from", states[col].first));
				cell.push_back(Pair("to", sm.GetTable()[row][col].first));
				transitions.push_back(cell);
			}
		}
	}
	return transitions;
}

CStateMachine & CStateMachineProcessor::Get(std::string const& id)
{
	auto it = std::find_if(m_stateMachines.begin(), m_stateMachines.end(),
		[id](CStateMachine const& sm) {return sm.GetId() == id; });
	if (it == m_stateMachines.end())
	{
		throw std::invalid_argument("Cannot find state machine with this id");
	}
	return *it;
}


void CStateMachineProcessor::TransferToMeale(CStateMachine & sm)
{
	if (sm.GetType() != "meale")
	{
		std::map<string, string> mooreStates;
		for (auto const & cell : sm.GetTable()[0])
		{
			mooreStates.insert({ cell.first, cell.second });
		}
		for (auto & row : sm.GetTable())
		{
			for (auto & cell : row)
			{
				auto it = mooreStates.find(cell.first);
				if (it != mooreStates.end())
				{
					cell.second = it->second;
				}
			}
		}
		for (auto & cell : sm.GetTable()[0])
		{
			cell.second = "";
		}
		sm.SetType("meale");
		m_jsonStateMachines.push_back(ToJson(sm));
	}

}
States CStateMachineProcessor::GetNewStates(StateTable const &meale)
{
	States mooreStates;
	int count = 0;
	for (auto & row : meale)
	{
		for (auto & cell : row)
		{
			if (!cell.second.empty()
				&& (find_if(mooreStates.begin(), mooreStates.end(),
					[&cell](auto const &state)
			{return state.second == cell; })
					== mooreStates.end()))
			{
				mooreStates.emplace("q" + to_string(count), cell);
				++count;
			}
		}
	}
	return mooreStates;
}

void CStateMachineProcessor::TransferToMoore(CStateMachine & sm)
{
	if (sm.GetType() == "moore")
	{
		return;
	}
	auto & smTable = sm.GetTable();
	auto mooreStates = GetNewStates(smTable);
	StateTable transferedTable(1);
	transferedTable[0].push_back({ "", "" });

	for (auto &state : mooreStates)
	{
		transferedTable[0].push_back(pair<string, string>(state.first, state.second.second));
	}

	for (size_t i = 1; i < smTable.size(); ++i)
	{
		auto vec = vector<pair<string, string>>(transferedTable[0].size());
		transferedTable.push_back(vec);
	}
	int k = 1;
	for (auto &input : smTable)
	{
		if (input[0] != make_pair<string, string>( "", "" ))
		{
			transferedTable[k][0] = input[0];
			++k;
		}
	}

	for (size_t i = 1; i < transferedTable[0].size(); ++i)
	{
		auto mooreState = mooreStates.at(transferedTable[0][i].first).first;
		for (size_t j = 0; j < smTable[0].size() - 1; ++j)
		{
			if (smTable[0][j].first == mooreState)
			{
				for (size_t k = 1; k < smTable.size(); ++k)
				{
					auto state2 = smTable[j][k];
					auto it = (find_if(mooreStates.begin(), mooreStates.end(),
						[&state2](auto const &state) {return state.second == state2; }));
					if (it != mooreStates.end())
					{
						transferedTable[j][k].first = it->first;
					}
				}
			}
		}
	}
	sm.GetTable() = transferedTable;
	sm.SetType("moore");
	m_jsonStateMachines.push_back(ToJson(sm));
}

CStateMachineProcessor::~CStateMachineProcessor()
{
}
