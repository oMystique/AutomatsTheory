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


//|||||||||||||||||||||||||Test;|||||||||||||||||||||||||//
void PrintOutputTable(vector<vector<pair<string, string>>> const &table, unsigned linesCount, unsigned columnsCount)
{
	for (int i = 0; i <= linesCount; ++i)
	{
		for (int j = 0; j != columnsCount; ++j)
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

vector<vector<pair<string, string>>> GetMealeTable(Object const &object)
{
	auto states = GetArray(object, STATES_TYPE);

	vector<vector<pair<string, string>>> table(1);
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

vector<vector<pair<string, string>>> GetMooreTable(Object const &object)
{
	auto table = vector<vector<pair<string, string>>>(1);
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

vector<Pair>* GetAutomats(string const &file_name)
{
	std::ifstream inputFile(file_name);

	Value value;
	read(inputFile, value);

	auto stateMachinesArray = value.get_obj().at(0).value_.get_array();
	auto firstSM = stateMachinesArray.at(0).get_obj();
	auto seconSM = stateMachinesArray.at(1).get_obj();
	auto firstType = firstSM.at(1).value_.get_str();
	auto secondType = seconSM.at(1).value_.get_str();

	cout << firstType << endl; //Example Meale;
	GetMealeTable(firstSM);

	cout << endl << endl;

	cout << secondType << endl; //Example Moore;
	GetMooreTable(seconSM);

	return{};
}
 

int main()
{
	GetAutomats("input.json");

    return 0;
}

