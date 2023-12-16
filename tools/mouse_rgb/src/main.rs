use clap::Parser;

mod cli;
mod graphics;
mod server;

fn main() {
	// Parse the command line
    let cli = cli::CommandLine::parse();

	let (producer, consumer) = std::sync::mpsc::channel();

	// Create the graphics thread
    let graphics_thread = std::thread::spawn(move || {
        graphics::graphics_thread(cli.size, producer);
    });

	// Create the server thread
	std::thread::spawn(move || {
		server::server_thread(cli.port, consumer);
	});

    graphics_thread.join().unwrap();
}
