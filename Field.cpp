#ifndef FIELD_HPP
#define FIELD_HPP

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <utility>

// The Field class represents a node with a name, a value, and optional subfields.
class Field {
public:
    // Data members
    std::string name;
    std::string value;
    std::vector<Field> subfields;
	bool delimiter = false;

    // Default constructor
    Field() = default;

    // Parameterized constructor using const references to avoid unnecessary copying.
    Field(const std::string& n, const std::string& v) : name(n), value(v) {}

    // Adds a subfield (lvalue version)
    void addField(const Field& subfield) {
        subfields.push_back(subfield);
    }

    // Adds a subfield (rvalue version, using move semantics)
    void addField(Field&& subfield) {
        subfields.push_back(std::move(subfield));
    }

    // Constructs a subfield in-place with provided arguments.
    template <typename... Args>
    void emplaceField(Args&&... args) {
        subfields.emplace_back(std::forward<Args>(args)...);
    }
	
	void addDelimiter() {
		Field field;
		field.delimiter = true;
		this->addField(field);
	}

    // Recursively generates a string representation of the Field with indentation.
    std::string print(const std::string tab = "  ", int tabulation = 0) const {
        std::stringstream ss;
		if (!this->delimiter) {
			for (int i = 0; i < tabulation; ++i)
				ss << tab;
			ss << name << ":";
			if (!value.empty())
				ss << " " << value;
		}
        ss << "\n";

        for (const auto& subfield : subfields)
            ss << subfield.print(tab, tabulation + (int)(!this->delimiter));
        return ss.str();
    }

    // Overload of the stream insertion operator for easy printing.
    friend std::ostream& operator<<(std::ostream& os, const Field& field) {
        os << field.print();
        return os;
    }
};

#endif // FIELD_HPP
