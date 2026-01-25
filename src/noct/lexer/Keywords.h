#pragma once

#include <string_view>
#include <unordered_map>

#include "noct/lexer/TokenType.h"

namespace Noct {

inline const std::unordered_map<std::string_view, TokenType> Keywords = {
	{ "and", TokenType::And },
	{ "class", TokenType::Class },
	{ "else", TokenType::Else },
	{ "false", TokenType::False },
	{ "for", TokenType::For },
	{ "fn", TokenType::Fn },
	{ "if", TokenType::If },
	{ "nil", TokenType::Nil },
	{ "or", TokenType::Or },
	{ "print", TokenType::Print },
	{ "return", TokenType::Return },
	{ "super", TokenType::Super },
	{ "this", TokenType::This },
	{ "true", TokenType::True },
	{ "make", TokenType::Make },
	{ "while", TokenType::While },
	{ "break", TokenType::Break },
	{ "maybe", TokenType::Maybe },
};

}
