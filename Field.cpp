// Field.cpp
#ifndef FIELD_CPP
#define FIELD_CPP
#define TABS "  "

#include <algorithm>
using namespace std;

class Field
{
public:
	string name = "";
	string value = "";
	vector<Field> subfields;
	bool colon = true;
	bool visible = true; // if false, just ignore it and move on

	Field(std::string n, std::string v, bool c = true) : name(n), value(v), colon(c) {}
	Field() {}

	void print(int tabulation = 0) {
		if (!this->visible) return;
		for (int i = 0; i < tabulation; ++i)
			cout << TABS;
		cout << this->name;
		if (this->colon) cout << ":";
		if (this->value != "") cout << " " << this->value;
		cout << endl;

		for (auto& field : this->subfields) field.print(tabulation+1);
		return;
	}

	void add_field(Field subfield) {
		this->subfields.push_back(subfield);
	}
};

// void print_fields(vector<Field> fields, int tabulation);

// void print_field(Field field, int tabulation = 0) {
	
// }

// void print_fields(vector<Field> fields, int tabulation = 0) {
// 	for (const auto& field : fields)
// 		print_field(field, tabulation);
// 	return;
// }

#endif