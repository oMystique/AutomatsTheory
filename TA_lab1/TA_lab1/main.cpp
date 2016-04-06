#include <vector>
#include <string>
#include "json_spirit/json_spirit.h"
#include <fstream>
#include <iostream>
#include <set>

using namespace std;
using namespace json_spirit;

static const string MEALE_TYPE = "meale";
static const string TRANSITIONS_TYPE = "transitions";
static const string STATES_TYPE = "states";


Array GetStatesArray(Object const &stateMachine, string const& key)
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

	auto states = GetStatesArray(object, STATES_TYPE);

	for (auto state : states)
	{
		statesArray.push_back(state.get_obj().at(0).value_.get_str()); //вектор состояние q1...q3
	}

	auto transitions = GetStatesArray(object, TRANSITIONS_TYPE);
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

