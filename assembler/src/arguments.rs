use clap::Parser;

#[derive(Parser, Debug)]
#[command(version, about)]
pub struct Arguments {
    #[arg(short, long, required = true, help = "Input .prism file for assembly")]
    pub input: String,
    #[arg(short, long, default_value = "out.bin", help = "Path to the output file")]
    pub output: String,
    #[arg(short, long, default_value = "false", help = "Show symbols being parsed")]
    pub symbols: bool,
}