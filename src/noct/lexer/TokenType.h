#pragma once

namespace Noct {

// clang-format off

enum class TokenType {
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    Comma, Dot, Minus, Plus, Semicolon, Slash, Star,
    QuestionMark, Colon,

    Bang, BangEqual,
    Equal, EqualEqual,
    Greater, GreaterEqual,
    Less, LessEqual,

    Identifier, String, Number,

    And, Class, Else, False, Fun, For, If, Nil, Or,
    Print, Return, Super, This, True, Var, While,

    Eof
};

// clang-format on

}
