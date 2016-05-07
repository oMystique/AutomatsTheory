#include "stdafx.h"
#include <vector>
#include <string>
#include "json_spirit/json_spirit.h"
#include <fstream>
#include <iostream>
#include <set>

using namespace std;
using namespace json_spirit;

static const string MEALE_TYPE = "meale";
static const string MOORE_TYPE = "moore";
static const string TRANSITIONS_TYPE = "transitions";
static const string STATES_TYPE = "states";
static const pair<string, string> EMPTY_CELL = { "", "" };

typedef vector<vector<pair<string, string>>> StateMachine;

//|||||||||||||||||||||||||Test;|||||||||||||||||||||||||//
void PrintOutputTable(StateMachine const &table, unsigned linesCount, unsigned columnsCount)
{
	for (unsigned i = 0; i <= linesCount; ++i)
	{
		for (unsigned j = 0; j != columnsCount; ++j)
		{
			if (table[i][j].first.empty())
			{
				cout << "      ";
			}
			else if (table[i][j].second.empty())
			{
				cout << "  " << table[i][j].first << "  ";
			}
			else
			{
				cout << table[i][j].first << "/" << table[i][j].second << " ";
			}
		}
		cout << endl;
	}
}
//|||||||||||||||||||||||||||||||||||||||||||||||||||||||//


Array GetArray(Object const &stateMachine, string const& key)
{
	Array statesArray;

	for (size_t i = 0; i < stateMachine.size(); ++i)
	{
		if (stateMachine[i].name_ == key)
		{
			return stateMachine[i].value_.get_array();
		}
	}
	return{};
}

StateMachine GetMealeTable(Object const &object)
{
	auto states = GetArray(object, STATES_TYPE);

	StateMachine table(1);
	table[0].push_back(EMPTY_CELL);

	for (auto state : states)
	{
		pair<string, string> cellState;
		cellState.first = state.get_obj().at(0).value_.get_str();
		cellState.second = "";
		table[0].push_back(cellState);

	}

	auto columnsCount = table[0].size();

	auto transitions = GetArray(object, TRANSITIONS_TYPE);
	vector<string > inputs;

	for (auto & transition : transitions)
	{
		auto input = transition.get_obj()[0].value_.get_str();
		auto output = transition.get_obj()[1].value_.get_str();
		auto from = transition.get_obj()[2].value_.get_str();
		auto to = transition.get_obj()[3].value_.get_str();
		auto iterator = std::find(inputs.begin(), inputs.end(), input);
		auto iter = find_if(table[0].begin(), table[0].end(), [&](auto const& pair)
		{
			return pair.first == from;
		});

		auto columnPos = iter - table[0].begin();

		if (iterator == inputs.end())
		{
			auto vec = vector<pair<string, string>>(columnsCount);
			vec[0] = { input, "" };
			vec[columnPos] = { to, output };
			table.push_back(vec);
			inputs.push_back(input);
		}
		else
		{
			auto rowWithInput = find_if(table.begin(), table.end(), [&](auto const& row)
			{
				return row[0].first == input;
			});
			rowWithInput->at(columnPos) = { to, output };
		}
	}

	PrintOutputTable(table, inputs.size(), columnsCount);

	return table;
}

//map<string, string> GetEmptyMap(size_t size)
//{
//	map<string, string> map;
//	for (size_t i = 0; i < size; i++)
//	{
//		map.emplace(EMPTY_CELL);
//	}
//	return map;
//} 
/* ??? */

StateMachine GetMooreTable(Object const &object)
{
	auto table = StateMachine(1);
	table[0].push_back(EMPTY_CELL);

	auto states = GetArray(object, STATES_TYPE);

	for (auto & state : states)
	{
		pair<string, string> cellState;
		cellState.first = state.get_obj()[0].value_.get_str();
		cellState.second = state.get_obj()[1].value_.get_str();
		table[0].push_back(cellState);
	}

	auto columnsCount = table[0].size();

	auto transitions = GetArray(object, TRANSITIONS_TYPE);
	vector<string> inputs;

	for (auto & transition : transitions)
	{
		auto input = transition.get_obj()[0].value_.get_str();
		auto from = transition.get_obj()[1].value_.get_str();
		auto to = transition.get_obj()[2].value_.get_str();
		auto iterator = std::find(inputs.begin(), inputs.end(), input);
		auto iter = find_if(table[0].begin(), table[0].end(), [&](auto const& pair)
		{
			return pair.first == from;
		});
		auto columnPos = iter - table[0].begin();
		if (iterator == inputs.end())
		{
			auto vec = vector<pair<string, string>>(columnsCount);
			vec[0] = { input, "" };
			vec[columnPos] = {to, ""};
			table.push_back(vec);
			inputs.push_back(input);
		}
		else
		{
			auto rowWithInput = find_if(table.begin(), table.end(), [&](auto const& row)
			{
				return row[0].first == input;
			});
			rowWithInput->at(columnPos) = { to, "" };
		}
	}

	PrintOutputTable(table, inputs.size(), columnsCount);

	return table;
}

StateMachine TransferToMeale(StateMachine const& moore)
{
	std::map<string, string> mooreStates;
	for (auto & cell : moore[0])
	{
		mooreStates.insert({cell.first, cell.second});
	}
	
	auto meale = moore;
	for (auto & row : meale)
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
	for (auto & cell : meale[0])
	{
		cell.second = "";
	}
	return meale;
}

map<string, pair<string, string>> GetNewStates(StateMachine const &meale)
{
	std::map<string, pair<string, string>> mooreStates;
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

StateMachine TransferToMoore(StateMachine const &meale)
{
	auto mooreStates = GetNewStates(meale);
	StateMachine transferedTable(1);
	transferedTable[0].push_back(EMPTY_CELL);

	for (auto &state : mooreStates)
	{
		transferedTable[0].push_back(pair<string, string>(state.first, state.second.second));
	}

	for (size_t i = 1; i < meale.size(); ++i)
	{
		auto vec = vector<pair<string, string>>(transferedTable[0].size());
		transferedTable.push_back(vec);
	}

	int k = 1;
	for (auto &input : meale)
	{
		if (input[0] != EMPTY_CELL)
		{
			transferedTable[k][0] = input[0];
			++k;
		}
	}

	for (size_t i = 1; i < transferedTable[0].size(); ++i)
	{
		auto mooreState = mooreStates.at(transferedTable[0][i].first).first;
		for (size_t j = 0; j < meale[0].size() - 1; ++j)
		{
			if (meale[0][j].first == mooreState)
			{
				for (size_t k = 1; k < meale.size(); ++k)
				{
					auto state2 = meale[j][k];
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

	cout << endl <<"Melee -> Moore" << endl;
	PrintOutputTable(transferedTable, transferedTable.size() - 1, transferedTable[0].size());

	return transferedTable;
}

vector<pair<StateMachine, string>> GetStateMachines(string const &file_name)
{
	vector<pair<StateMachine, string>> stateMachines;
	std::ifstream inputFile(file_name);

	Value value;
	read(inputFile, value);

	auto stateMachinesArray = value.get_obj().at(0).value_.get_array();
	auto firstSM = stateMachinesArray.at(0).get_obj();
	auto seconSM = stateMachinesArray.at(1).get_obj();
	auto firstType = firstSM.at(1).value_.get_str();
	auto secondType = seconSM.at(1).value_.get_str();

	cout << firstType << endl; //Example Meale;
	stateMachines.push_back({ GetMealeTable(firstSM), "meale" });

	cout << endl << endl;

	cout << secondType << endl; //Example Moore;
	stateMachines.push_back({GetMooreTable(seconSM), "moore"});

	return stateMachines;
}
 

int main()
{
	auto stateMachines = GetStateMachines("input.json");
	auto meale = TransferToMeale(stateMachines[1].first);
	cout << endl << "Moore -> Melee"  << endl;
	PrintOutputTable(meale, 2, 4);
	TransferToMoore(stateMachines[0].first);
    return 0;
}

