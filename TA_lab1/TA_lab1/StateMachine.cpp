#include "StateMachine.h"
using namespace std;
using namespace json_spirit;

CStateMachine::CStateMachine(json_spirit::Object const& smData)
{
	auto type = smData.at(1).value_.get_str();
	m_id = smData.at(0).value_.get_str();
	if (type == "meale")
	{
		m_table = GetMealeTable(smData);
		m_type = "meale";
	}
	else if (type == "moore")
	{
		m_table = GetMooreTable(smData);
		m_type = "moore";
	}
}

StateTable & CStateMachine::GetTable()
{
	return m_table;
}

StateTable const& CStateMachine::GetTable() const
{
	return m_table;
}

std::string const& CStateMachine::GetType() const
{
	return m_type;
}

void CStateMachine::SetType(std::string const& type)
{
	m_type = type;
}

std::string CStateMachine::GetId() const
{
	return m_id;
}

Array CStateMachine::GetArray(Object const &stateMachine, string const& key)
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

StateTable CStateMachine::GetMealeTable(Object const &sm)
{
	auto states = GetArray(sm, "states");

	StateTable table(1);
	table[0].push_back({"", ""});

	for (auto state : states)
	{
		pair<string, string> cellState;
		cellState.first = state.get_obj().at(0).value_.get_str();
		cellState.second = "";
		table[0].push_back(cellState);

	}

	auto columnsCount = table[0].size();

	auto transitions = GetArray(sm, "transitions");
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

	return table;
}

StateTable CStateMachine::GetMooreTable(Object const &sm)
{
	auto table = StateTable(1);
	table[0].push_back({"", ""});

	auto states = GetArray(sm, "states");

	for (auto & state : states)
	{
		pair<string, string> cellState;
		cellState.first = state.get_obj()[0].value_.get_str();
		cellState.second = state.get_obj()[1].value_.get_str();
		table[0].push_back(cellState);
	}

	auto columnsCount = table[0].size();

	auto transitions = GetArray(sm, "transitions");
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
			vec[columnPos] = { to, "" };
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