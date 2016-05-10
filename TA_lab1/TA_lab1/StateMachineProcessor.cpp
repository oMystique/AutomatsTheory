#include "stdafx.h"
#include "StateMachineProcessor.h"

using namespace json_spirit;
using namespace std;

bool operator ==(vector<pair<string, string>> const &v1, vector<pair<string, string>> const &v2)
{
	if (v1.size() != v2.size())
	{
		return false;
	}

	for (size_t i = 0; i != v1.size(); ++i)
	{
		if (v1[i].first != v2[i].first)
		{
			return false;
		}
	}
	return true;
}

vector<pair<string, string>> GetColumn(StateTable const &sm, size_t columnNumber)
{
	vector<pair<string, string>> v;
	for (auto it = sm.begin() + 1; it != sm.end(); ++it)
	{
		v.push_back((*it)[columnNumber]);
	}
	return v;
}

CStateMachineProcessor::CStateMachineProcessor(std::string const& input, std::string const& output)
	: m_isFirstTimeWrite(true)
	, m_output(output)
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

void CStateMachineProcessor::WriteToFile()
{
	ofstream out(m_output);
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

CStateMachine & CStateMachineProcessor::Get(string const& id)
{
	auto it = find_if(m_stateMachines.begin(), m_stateMachines.end(),
		[id](CStateMachine const& sm) {return sm.GetId() == id; });
	if (it == m_stateMachines.end())
	{
		throw invalid_argument("Cannot find state machine with this id");
	}
	return *it;
}


void CStateMachineProcessor::TransferToMeale(CStateMachine & sm)
{
	if (sm.GetType() != "meale")
	{
		map<string, string> mooreStates;
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

void CStateMachineProcessor::Determine(CStateMachine & sm)
{
	StateTable dTable;
	dTable.push_back({ {"", ""}, {sm.GetTable()[0][1]} });
	for (size_t i = 1; i < sm.GetTable().size(); i++)
	{
		dTable.push_back({ { sm.GetTable()[i][0] } });
	}
	auto startSize = dTable[0].size();
	size_t addedStatesCount = 0;
	for (size_t col = 1; col < startSize + addedStatesCount; col++)
	{
		for (size_t row = 1; row < dTable.size(); row++)
		{
			//split state -from- and add to cell -to- each of result
			//if -to- state is new then add it to dTable states and addedStatesCount++
			vector<string> splittedState;
			boost::split(splittedState, dTable[0][col].first, bind2nd(equal_to<char>(), ';'));
			pair<string, string> cell;
			for (auto & state : splittedState)
			{
				auto it = find_if(sm.GetTable()[0].begin(), sm.GetTable()[0].end(),
					[&](pair<string, string> const& pair) {return pair.first == state; });
				auto pos = it - sm.GetTable()[0].begin();
				if (sm.GetTable()[row][pos].first != ""
					&& cell.first.find(sm.GetTable()[row][pos].first) == string::npos)
				{
					cell.first += sm.GetTable()[row][pos].first;
					cell.first += ";";
				}
			}
			dTable[row].push_back(cell);
			auto & cellFirst = dTable[row][col].first;
			cellFirst.erase(cellFirst.end() - 1, cellFirst.end());
			vector<string> compareVec;
			boost::split(compareVec, cellFirst, bind2nd(equal_to<char>(), ';'));
			std::sort(compareVec.begin(), compareVec.end());
			auto it = find_if(dTable[0].begin(), dTable[0].end(),
				[&](pair<string, string> const& pair) {
				vector<string> compare2;
				boost::split(compare2, pair.first, bind2nd(equal_to<char>(), ';'));
				std::sort(compare2.begin(), compare2.end());
				return compare2 == compareVec;
			});
			if (it == dTable[0].end())
			{
				dTable[0].push_back({ cellFirst, "" });
				addedStatesCount++;
			}
		}
	}

	map<string, string> newStates;
	auto stateName = "q";
	for (size_t i = 1; i < dTable[0].size(); i++)
	{
		vector<string> state;
		boost::split(state, dTable[0][i].first, bind2nd(equal_to<char>(), ';'));
		sort(state.begin(), state.end());
		string strState;
		strState = accumulate(state.begin(), state.end(), strState);
		newStates.emplace(strState, stateName + to_string(i));
	}

	for (size_t row = 0; row < dTable.size(); row++)
	{
		for (size_t col = 1; col < dTable[0].size(); col++)
		{

			vector<string> state;
			boost::split(state, dTable[row][col].first, bind2nd(equal_to<char>(), ';'));
			sort(state.begin(), state.end());
			string strState;
			strState = accumulate(state.begin(), state.end(), strState);
			dTable[row][col].first = newStates.at(strState);
		}
	}
	sm.GetTable() = dTable;
	m_jsonStateMachines.push_back(ToJson(sm));
}

StateTable CStateMachineProcessor::AllocateOfEquivalenceClass(StateTable resourceST, StateTable const & originalST)
{
	int classNumber = 1;
	map<string, int> classes;
	while (resourceST[0].size() != 1)
	{
		auto currentColumn = GetColumn(resourceST, 1);
		classes.emplace(resourceST[0][1].first, classNumber);
		for (auto i = 0; i != resourceST.size(); ++i)
		{
			auto &vec = resourceST[i];
			vec.erase(vec.begin() + 1);
		}
		for (size_t i = 1; i < resourceST[0].size(); ++i)
		{
			auto cmpColumn = GetColumn(resourceST, i);
			if (cmpColumn == currentColumn)
			{
				classes.emplace(resourceST[0][i].first, classNumber);
			}
		}
		++classNumber;
	}

	auto outputST = originalST;
	for (auto i = 1; i != outputST.size(); ++i)
	{
		for (auto j = 1; j != outputST[0].size(); ++j)
		{
			auto state = classes.find(originalST[i][j].first);
			if (state != classes.end())
			{
				outputST[i][j].first = to_string(state->second);
			}
		}
	}
	return outputST;
}

void CStateMachineProcessor::Minimize(CStateMachine & sm)
{
	if (sm.GetType() != "melee")
	{
		TransferToMeale(sm);
	}

	//for swap Y and Q, for example
	auto tempCopyST = sm.GetTable();

	for (unsigned i = 1; i <= tempCopyST.size() - 1; ++i)
	{
		for (unsigned j = 1; j != tempCopyST[0].size(); ++j)
		{
			swap(tempCopyST[i][j].first, tempCopyST[i][j].second);
		}
	}
	//

	StateTable oldSTCpy = AllocateOfEquivalenceClass(tempCopyST, sm.GetTable());
	StateTable currentSTCpy;
	do
	{
		currentSTCpy = AllocateOfEquivalenceClass(oldSTCpy, sm.GetTable());
		oldSTCpy = currentSTCpy;

	} while (oldSTCpy != currentSTCpy);

	sm.GetTable() = oldSTCpy;
	m_jsonStateMachines.push_back(ToJson(sm));
}

CStateMachineProcessor::~CStateMachineProcessor()
{
}
