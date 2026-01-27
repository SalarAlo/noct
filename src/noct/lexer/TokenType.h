#pragma once

namespace Noct {

// clang-format off

enum class TokenType {
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    Comma, Dot, Minus, MinusMinus, Plus, PlusPlus, Semicolon, Slash, Star,
    QuestionMark, Colon, Percentage,

    Bang, BangEqual,
    Equal, EqualEqual,
    Greater, GreaterEqual,
    Less, LessEqual,

    Identifier, String, Number,

    And, Class, Else, False, Fn, For, If, Nil, Or,
    Print, Return, Super, This, True, Var, While, Break,
    Maybe, Import,

    Eof
};

// clang-format on

}
