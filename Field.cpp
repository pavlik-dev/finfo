// field.cpp
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

	Field(std::string n, std::string v, bool c = true) : name(n), value(v), colon(c) {}
	Field() {}
};

Field quick_field(string name = "", string value = "", bool colon = true){
	Field _field(name, value, colon);
	return _field;
}

void print_fields(vector<Field> fields, int tabulation);

void print_field(Field field, int tabulation = 0) {
	for (int i = 0; i < tabulation; ++i)
		cout << TABS;
	cout << field.name;
	if (field.colon) cout << ":";
	if (field.value != "") cout << " " << field.value;
	cout << endl;
	print_fields(field.subfields, tabulation+1);
	return;
}

void print_fields(vector<Field> fields, int tabulation = 0) {
	for (const auto& field : fields)
		print_field(field, tabulation);
	return;
}

#endif