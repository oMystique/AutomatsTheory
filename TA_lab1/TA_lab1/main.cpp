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

vector<Pair>* ParseMeale(Object const &object)
{
	vector<string > statesArray;

	auto states = GetArray(object, STATES_TYPE);

	for (auto state : states)
	{
		statesArray.push_back(state.get_obj().at(0).value_.get_str()); //вектор состояние q1...q3
	}

	auto transitions = GetArray(object, TRANSITIONS_TYPE);
	for (auto transition : transitions)
	{
		cout << endl;
		cout << transition.get_obj().at(0).value_.get_str() << endl;
		cout << transition.get_obj().at(1).value_.get_str() << endl;
		cout << transition.get_obj().at(2).value_.get_str() << endl;
		cout << transition.get_obj().at(3).value_.get_str() << endl;
	}

	return {};
}

map<string, string> GetEmptyMap(size_t size)
{
	map<string, string> map;
	for (size_t i = 0; i < size; i++)
	{
		map.emplace(EMPTY_CELL);
	}
	return map;
}


vector<vector<pair<string, string>>> GetMooreTable(Object const &object)
{
	auto table = vector<vector<pair<string, string>>>(1);
	table[0].push_back(EMPTY_CELL);
	vector<string> inputs;

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
	auto seconType = seconSM.at(1).value_.get_str();

	cout << firstType << endl;
	GetMooreTable(seconSM);
	if (firstType == MEALE_TYPE)
	{
		ParseMeale(firstSM);
	}

	return{};
}
 

int main()
{
	GetAutomats("input.json");

    return 0;
}

