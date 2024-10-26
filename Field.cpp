// Field.cpp
#ifndef FIELD_CPP
#define FIELD_CPP
#define TABS "  "

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include "exception.cpp"

using namespace std;

// #pragma region Exceptions

class DuplicateIDException {
private:
	string _what;
public:
	DuplicateIDException(string what) : _what(what) {}
	string what() {
		return this->_what;
	}
};

// #pragma endreg

class Field
{
public:
	string id = "";  // can be empty ig
	string name = "";
	string value = "";
	map<string, Field> subfields;
	bool colon = true;
	bool visible = true; // if false, just ignore it and move on


	// Constructors
	Field(std::string id, std::string n, std::string v, bool c = true):
		id(id), name(n), value(v), colon(c) {}
	//Field(std::string n, std::string v, bool c = true):
	//	name(n), value(v), colon(c) {}
	// const Field no_id(std::string n, std::string v, bool c = true) {
	// 	return Field("", n, v, c);
	// }

	Field() {}

	// tabulation - specifies the level of in(d/t??)ent used
	// show_id - showes field's id instead of name
	void print(int tabulation = 0, bool show_id = false, string id_pre = "") {
		if (!this->visible) return;
		for (int i = 0; i < tabulation; ++i)
			cout << TABS;
		if (!show_id) cout << this->name;
		else cout << id_pre << this->id;
		if (this->colon) cout << ":";
		if (this->value != "") cout << " " << this->value;
		cout << endl;

		for (auto& field : this->subfields)
			field.second.print(tabulation+1, show_id, id_pre+this->id+".");
		return;
	}

	void add_field(Field subfield) {
		if (this->subfields.count(subfield.id) > 0) {
			throw new DuplicateIDException(subfield.id);
		}
		this->subfields[subfield.id] = subfield;
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