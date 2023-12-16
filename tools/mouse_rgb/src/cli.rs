use clap::Parser;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
pub struct CommandLine {
	#[arg(short, long, default_value_t = 2425)]
    pub port: u16,
    #[arg(long, default_value_t = 800)]
    pub size: u32,
}
