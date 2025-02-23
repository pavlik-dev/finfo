// Field.cpp
#ifndef FIELD_CPP
#define FIELD_CPP
#define TABS "  "  //Change this however you want

#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Field
{
public:
	string name = "";
	string value = "";
	vector<Field> subfields;

	// Constructors
	Field(std::string n, std::string v):
		name(n), value(v) {}

	// tabulation - specifies the level of indent used
	// show_id - showes field's id instead of name
	void print(int tabulation = 0) {
		for (int i = 0; i < tabulation; ++i)
			cout << TABS;
		cout << this->name << ":";
		if (this->value != "") cout << " " << this->value;
		cout << endl;

		for (size_t i = 0; i < this->subfields.size(); ++i)
            this->subfields[i].print(tabulation+1);
		return;
	}

	void add_field(Field subfield) {
		this->subfields.push_back(subfield);
	}
};

#endif
