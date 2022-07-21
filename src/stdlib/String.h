#ifndef STRING_H
#define STRING_H

#include "Stdlib.h"
// Class String
class String {
  // Prototype for '+'
  // operator overloading
  friend String operator+(const String& lhs, const String& rhs);
  char* str;

 public:
  // No arguments constructor
  String();

  // pop_back() function
  void pop_bk();

  // push_back() function
  void push_bk(char a);

  // To get the length
  int get_length();

  const char* get() { return str; }
  // Function to copy the string
  // of length len from position pos
  void copy(char s[], int len, int pos);

  // Swap strings function
  void swp(String& rhs);

  // Constructor with 1 arguments
  String(const char* val);

  // Copy Constructor
  String(const String& source);

  // Move Constructor
  String(String&& source);

  // Overloading the assignment
  // operator
  String& operator=(const String& rhs);

  // Destructor
  ~String() { delete str; }
};

// Overloading the assignment operator
String& String::operator=(const String& rhs) {
  if (this == &rhs) return *this;
  delete[] str;
  str = new char[Std::strlen(rhs.str) + 1];
  Std::strcpy(str, rhs.str);
  return *this;
}

// Overloading the plus operator
String operator+(const String& lhs, const String& rhs) {
  int length = Std::strlen(lhs.str) + Std::strlen(rhs.str);

  char* buff = new char[length + 1];

  // Copy the strings to buff[]
  Std::strcpy(buff, lhs.str);
  Std::strcat(buff, rhs.str);
  buff[length] = '\0';

  // String temp
  String temp{buff};

  // delete the buff[]
  delete[] buff;

  // Return the concatenated string
  return temp;
}

// Function for swapping string
void String::swp(String& rhs) {
  String temp{rhs};
  rhs = *this;
  *this = temp;
}

// Function to copy the string
void String::copy(char s[], int len, int pos) {
  for (int i = 0; i < len; i++) {
    s[i] = str[pos + i];
  }
  s[len] = '\0';
}

// Function to implement push_bk
void String::push_bk(char a) {
  // Find length of string
  int length = Std::strlen(str);

  char* buff = new char[length + 2];

  // Copy character from str
  // to buff[]
  for (int i = 0; i < length; i++) {
    buff[i] = str[i];
  }
  buff[length] = a;
  buff[length + 1] = '\0';

  // Assign the new string with
  // char a to string str
  *this = String{buff};

  // Delete the temp buff[]
  delete[] buff;
}

// Function to implement pop_bk
void String::pop_bk() {
  int length = Std::strlen(str);
  char* buff = new char[length];

  // Copy character from str
  // to buff[]
  for (int i = 0; i < length - 1; i++) buff[i] = str[i];
  buff[length - 1] = '\0';

  // Assign the new string with
  // char a to string str
  *this = String{buff};

  // delete the buff[]
  delete[] buff;
}

// Function to implement get_length
int String::get_length() { return Std::strlen(str); }

// Function to illustrate Constructor
// with no arguments
String::String() : str{nullptr} {
  str = new char[1];
  str[0] = '\0';
}

// Function to illustrate Constructor
// with one arguments
String::String(const char* val) {
  if (val == nullptr) {
    str = new char[1];
    str[0] = '\0';
  }

  else {
    str = new char[Std::strlen(val) + 1];

    // Copy character of val[]
    // using Std::strcpy
    Std::strcpy(str, val);
    str[Std::strlen(val)] = '\0';
  }
}

// Function to illustrate
// Copy Constructor
String::String(const String& source) {
  str = new char[Std::strlen(source.str) + 1];
  Std::strcpy(str, source.str);
}

// Function to illustrate
// Move Constructor
String::String(String&& source) {
  str = source.str;
  source.str = nullptr;
}

#endif