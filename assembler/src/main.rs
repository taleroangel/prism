mod lexer;
mod reader;
mod parser;
mod arguments;
mod writer;

use anyhow::Result;
use clap::Parser;

fn main() -> Result<()> {
    // Get the command line arguments
    let cmdargs = arguments::Arguments::parse();

    // Read an prepare the source code
    let source = reader::readpf(cmdargs.input.as_str())?;
    println!("Successfully read ({}) symbols from file `{}`", source.len(), cmdargs.input);

    // Create a lexer for the code
    let mut lexer = lexer::create_lexer(source.as_str());
    // Identify the labels
    let labels = parser::identify_labels(source.as_str())?;
    println!("Identified ({}) labels, [{:?}]", labels.len(), labels.keys());

    // Parse the tokens into source code
    let instructions = parser::parsepf(&mut lexer, labels)?;
    println!("Parsed source assembly, ({}) instructions found", instructions.len());

    // Show symbols for debugging
    if cmdargs.symbols {
        println!("Showing parsed symbols");
        for (it, &instruction) in instructions.iter().enumerate() {
            let bytes: Vec<u8> = instruction.into();
            println!("{it} -> {instruction:?} -> {bytes:02X?}")
        }
    }

    // Assemble the instructions
    let assembled = libnewton::binary::assemble(&instructions);
    println!("Assembled source code into Prism Binary Format");

    // Write to the file
    writer::writebin(cmdargs.output.as_str(), &assembled)?;
    println!("Assembly succeeded, ({}) bytes written", assembled.len());

    Ok(())
}