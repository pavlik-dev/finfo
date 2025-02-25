// Field.cpp
#ifndef FIELD_CPP
#define FIELD_CPP
#define TABS "  " // Change this however you want

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

class Field
{
public:
	string _name;
	string value;
	vector<Field> subfields;

	// Constructors
	Field(std::string n, std::string v) : _name(n), value(v) {}

	// tabulation - specifies the level of indent used
	string print(int tabulation = 0)
	{
		stringstream test;
		for (int i = 0; i < tabulation; ++i)
			test << TABS;
		test << this->_name << ":";
		if (this->value != "")
			test << " " << this->value;
		test << endl;

		for (size_t i = 0; i < this->subfields.size(); ++i)
			test << this->subfields[i].print(tabulation + 1);
		return test.str();
	}

	void add_field(Field subfield)
	{
		this->subfields.push_back(subfield);
	}
};

#endif
