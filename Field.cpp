#ifndef FIELD_HPP
#define FIELD_HPP

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

// The Field class represents a node with a name, a value, and optional subfields.
class Field
{
public:
  // Data members
  std::string name;
  std::string value;
  std::vector<Field> subfields;
  bool delimiter;

  // Default constructor
  Field(bool delim) : delimiter(delim) {}

  // Parameterized constructor using const references to avoid unnecessary copying.
  Field(const std::string &n, const std::string &v) : name(n), value(v), delimiter(false) {}

  // Adds a subfield (lvalue version)
  void addField(const Field &subfield)
  {
    subfields.push_back(subfield);
  }

  // Removed the rvalue reference version method for C++98 compatibility.

  void emplaceField(const std::string &name, const std::string &value)
  {
    this->addField(Field(name, value));
  }

  // Adds a delimiter field.
  void addDelimiter()
  {
    Field field(true);
    this->addField(field);
  }

  // Recursively generates a string representation of the Field with indentation.
  std::string print(const std::string &tab = "  ", int tabulation = 0) const
  {
    std::stringstream ss;
    if (!this->delimiter)
    {
      for (int i = 0; i < tabulation; ++i)
        ss << tab;
      ss << name << ":";
      if (!value.empty())
        ss << " " << value;
    }
    ss << "\n";

    // Use an iterator-based loop instead of a range-based for loop.
    for (std::vector<Field>::const_iterator it = subfields.begin(); it != subfields.end(); ++it)
      ss << it->print(tab, tabulation + (int)(!this->delimiter));
    return ss.str();
  }

  // Overload of the stream insertion operator for easy printing.
  friend std::ostream &operator<<(std::ostream &os, const Field &field)
  {
    os << field.print();
    return os;
  }
};

#endif // FIELD_HPP