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


Array GetTypeArray(Object const &obj, string const &typeObject)
{
	Array statesArray;

	for (size_t i = 0; i < obj.size(); ++i)
	{
		if (obj[i].name_ == typeObject)
		{
			return obj[i].value_.get_array();
		}
	}
	
	return{};
}

vector<Pair>* ParseMeale(Object const &object)
{
	vector<string > statesArray;

	auto states = GetTypeArray(object, STATES_TYPE);

	for (auto state : states)
	{
		statesArray.push_back(state.get_obj().at(0).value_.get_str()); //вектор состояние q1...q3
	}

	auto transitions = GetTypeArray(object, TRANSITIONS_TYPE);
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

	auto automats = value.get_obj().at(0).value_.get_array();

	auto type = automats.at(0).get_obj().at(1).value_.get_str();

	cout << type << endl;

	if (type == MEALE_TYPE)
	{
		ParseMeale(automats.at(0).get_obj());
	}

	return{};
}
 

int main()
{
	GetAutomats("input.json");

    return 0;
}

