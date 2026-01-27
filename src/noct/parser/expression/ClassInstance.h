#pragma once

#include "noct/parser/expression/ClassValue.h"

namespace Noct {
struct ClassInstance {
	ClassValueRef ClassReference;
};
inline bool operator==(const ClassInstance& a, const ClassInstance& b) {
	return a.ClassReference == b.ClassReference;
}

inline bool operator!=(const ClassInstance& a, const ClassInstance& b) {
	return !(a == b);
}

}
