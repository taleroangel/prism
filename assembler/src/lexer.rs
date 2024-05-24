use logos::{Lexer, Logos};

/// Definition of all available lexer tokens\
/// Input text *must be lowercase* and must not contain comments, comments are not yet supported
#[derive(Logos, Debug)]
#[logos(skip r"[ \t\f]+")]
pub enum PrismToken {
    #[regex(r"[a-z]+", PrismToken::stringify, priority = 1)]
    Text(String),

    #[regex(r"[0-9]+", PrismToken::number)]
    #[regex(r"#[a-f0-9]+", PrismToken::hex_number)]
    Number(u8),

    #[regex(r"[a-z0-9]+:", PrismToken::suffix_stringify, priority = 2)]
    LabelDeclaration(String),

    #[regex(r"\$[a-z]{2}", PrismToken::prefix_stringify)]
    Register(String),

    #[regex(r"\$[0-9]+", PrismToken::prefix_number)]
    GeneralPurposeRegister(u8),

    #[token("\n")]
    Newline,

    #[token("(")]
    ArrayBegin,

    #[token(")")]
    ArrayEnd,

    #[token(";")]
    LineComment,
}

/// Creates a new lexer for a given source code
pub fn create_lexer(source: &str) -> Lexer<PrismToken> {
    PrismToken::lexer(source)
}

/// Contains all the parsers for the input regex in a [PrismToken]
impl PrismToken {
    /// Parse a token into an explicit String
    fn stringify(lexer: &mut Lexer<Self>) -> Option<String> {
        lexer.slice().parse().ok()
    }

    // Remove suffix from string
    fn suffix_stringify(lexer: &mut Lexer<Self>) -> Option<String> {
        let slice = lexer.slice();
        let strnum = &slice[..slice.len() - 1];
        strnum.parse().ok()
    }

    /// Remove prefix from string
    fn prefix_stringify(lexer: &mut Lexer<Self>) -> Option<String> {
        let slice = lexer.slice();
        let strnum = &slice[1..slice.len()];
        strnum.parse().ok()
    }

    /// Parse text as [u8]
    fn number(lexer: &mut Lexer<Self>) -> Option<u8> {
        lexer.slice().parse().ok()
    }

    /// Parse the hex format beginning with # into a [u8]
    fn hex_number(lexer: &mut Lexer<Self>) -> Option<u8> {
        let slice = lexer.slice();
        let strnum = &slice[1..slice.len()];
        Some(u8::from_str_radix(strnum, 16).ok()?)
    }

    /// Remove a prefix from a string and transform it to [u8]
    fn prefix_number(lexer: &mut Lexer<Self>) -> Option<u8> {
        let slice = lexer.slice();
        let strnum = &slice[1..slice.len()];
        strnum.parse().ok()
    }
}