#pragma once

#include <exception>

#include "noct/lexer/NoctObject.h"

namespace Noct {

class ReturnException : public std::exception {
public:
	ReturnException(const NoctObject& obj)
	    : m_Object(obj) { };

	const NoctObject& GetObject() const { return m_Object; }

private:
	NoctObject m_Object;
};

}
