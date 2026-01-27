#pragma once

#include "noct/lexer/NoctObject.h"

namespace Noct {
struct RunResult {
	bool HadParseError = false;
	bool HadSemanticError = false;
	bool HadRuntimeError = false;
	NoctObject Value {};
};

}
