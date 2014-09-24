#pragma once

// The MIT License (MIT)
//
// Copyright (c) 2014 Siyuan Ren (netheril96@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifdef CODEGEN
#define ANNOTATE(x) __attribute__((annotate(x)))
#else
#define ANNOTATE(x) 
#endif

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>

using namespace std;    // This is terrible practice in header files; used here to demonstrate the parsing capabilities
                        // Don't do this in production code!

struct Date {
  int year ANNOTATE("<codegen><required>true</required></codegen>");
  int month ANNOTATE("<codegen><required>true</required></codegen>");
  int day ANNOTATE("<codegen><required>true</required></codegen>");

  explicit Date() : year(), month(), day() {}

  bool operator==(Date d) const {
    return year == d.year && month == d.month && day == d.day;
  }

  bool operator!=(Date d) const { return !(*this == d); }
} ANNOTATE("<codegen><strict_parsing>true</strict_parsing></codegen>");

namespace config {
namespace event {

struct BlockEvent {

  uint64_t serial_number
  ANNOTATE("<codegen><required>true</required></codegen>");

  unsigned long long admin_ID
  ANNOTATE("<codegen><key>administrator ID</key></codegen>");

  Date date;
  string description;
  string details;

  explicit BlockEvent()
      : serial_number(), admin_ID(255), date(),
        description("\x2f\x2a\x20\x69\x6e\x69\x74\x20\x2a\x2f\x20\x74\x72\x79"
                    "\x69\x6e\x67\x20\x74\x6f\x20\x6d\x65\x73\x73\x20\x75\x70"
                    "\x20\x77\x69\x74\x68\x20\x74\x68\x65\x20\x63\x6f\x64\x65"
                    "\x20\x67\x65\x6e\x65\x72\x61\x74\x6f\x72"),
        details() {
    date.year = 1970;
    date.month = 1;
    date.day = 1; /* Assign date to the UNIX epoch */
  }
};

struct User {

  unsigned long long ID
  ANNOTATE("<codegen><required>true</required></codegen>");

  string nickname ANNOTATE("<codegen><required>true</required></codegen>");

  Date birthday;
  shared_ptr<event::BlockEvent> block_event;
  vector<event::BlockEvent> dark_history;
  map<string, string> optional_attributes;

  explicit User()
      : ID(), nickname("\xe2\x9d\xb6\xe2\x9d\xb7\xe2\x9d\xb8"), birthday(),
        block_event(), dark_history(), optional_attributes() {}
};
}
}
