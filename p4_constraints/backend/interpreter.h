/*
 * Copyright 2020 The P4-Constraints Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// The interpreter evaluates a constraint with respect to a table entry.

#ifndef P4_CONSTRAINTS_BACKEND_INTERPRETER_H_
#define P4_CONSTRAINTS_BACKEND_INTERPRETER_H_

#include <gmpxx.h>

#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "p4/v1/p4runtime.pb.h"
#include "p4_constraints/ast.pb.h"
#include "p4_constraints/backend/constraint_info.h"

namespace p4_constraints {

// Checks if a given table entry satisfies the entry constraint attached to its
// associated table. Returns true if this is the case or if no constraint
// exists. Returns an InvalidArgument Status if the entry belongs to a table not
// present in ConstraintInfo, or if it is inconsistent with the table definition
// in ConstraintInfo.
absl::StatusOr<bool> EntryMeetsConstraint(const p4::v1::TableEntry& entry,
                                          const ConstraintInfo& context);

// -- END OF PUBLIC INTERFACE --------------------------------------------------

// Exposed for testing only.
namespace internal_interpreter {

// -- Runtime representations --------------------------------------------------

// We use mpz_class to represent all integers, including arbitrary-precision
// and fixed-width signed/unsigned integers.
using Integer = mpz_class;

struct Exact {
  Integer value;
};

struct Ternary {
  Integer value;
  Integer mask;
};

struct Lpm {
  Integer value;
  Integer prefix_length;
};

struct Range {
  Integer low;
  Integer high;
};

inline bool operator==(const Exact& left, const Exact& right) {
  return left.value == right.value;
}

inline bool operator==(const Ternary& left, const Ternary& right) {
  return left.value == right.value && left.mask == right.mask;
}

inline bool operator==(const Lpm& left, const Lpm& right) {
  return left.value == right.value && left.prefix_length == right.prefix_length;
}

inline bool operator==(const Range& left, const Range& right) {
  return left.low == right.low && left.high == right.high;
}

// Evaluation can result in a value of various types.
// We use a tagged union to ease debugging (see DynamicTypeCheck); an untagged
// union would work just fine assuming the type checker has no bugs.
using EvalResult = absl::variant<bool, Integer, Exact, Ternary, Lpm, Range>;

// Parsed representation of p4::v1::TableEntry.
struct TableEntry {
  std::string table_name;
  absl::flat_hash_map<std::string, EvalResult> keys;
  // TODO(smolkaj): once we support actions, they will be added here.
};

absl::StatusOr<EvalResult> Eval(const ast::Expression& expr,
                                const TableEntry& entry);

}  // namespace internal_interpreter

}  // namespace p4_constraints

#endif  // P4_CONSTRAINTS_BACKEND_INTERPRETER_H_
