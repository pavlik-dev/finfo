// Field.cpp
#ifndef FIELD_CPP
#define FIELD_CPP
#define TABS "  "

#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

class Field
{
public:
	string id = "";  // can be empty ig
	string name = "";
	string value = "";
	vector<Field> subfields;
	bool colon = true;
	bool visible = true; // if false, just ignore it and move on


	// Constructors
	Field(std::string id, std::string n, std::string v, bool c = true) : id(id), name(n), value(v), colon(c) {}
	//Field(std::string n, std::string v, bool c = true) : name(n), value(v), colon(c) {}
	const Field no_id(std::string n, std::string v, bool c = true) {
		return Field("", n, v, c);
	}

	Field() {}

	// tabulation - specifies the level of in(d/t??)ent used
	// show_id - showes field's id instead of name
	void print(int tabulation = 0, bool show_id = false) {
		if (!this->visible) return;
		for (int i = 0; i < tabulation; ++i)
			cout << TABS;
		if (!show_id) cout << this->name;
		else cout << this->id;
		if (this->colon) cout << ":";
		if (this->value != "") cout << " " << this->value;
		cout << endl;

		for (auto& field : this->subfields) field.print(tabulation+1, show_id);
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