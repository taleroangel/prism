#[macro_use]
extern crate log;

use clap::Parser;
use libnewton::{
    core::instruction::Instruction,
    interpreter::{BuffItem, Interpreter, InterpreterAction},
};

use std::{
    io::Read,
    net::{Shutdown, SocketAddr, TcpListener, TcpStream},
    sync::mpsc::{self, Receiver, Sender},
    thread,
    time::Duration,
};

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct CommandLine {
    #[arg(short, long)]
    number_of_leds: u8,
    #[arg(short, long, default_value_t = 2425)]
    port: u16,
    #[arg(short, long, default_value_t = 255)]
    size: u16,
}

fn main() {
    pretty_env_logger::init();

    /* Parse command line arguments */
    let command_line = CommandLine::parse();
    trace!(target: "main", "Initialized with arguments: {:?}", command_line);

    /* Create the comunication channels */
    let (socket_interpreter_tx, socket_interpreter_rx): (Sender<u16>, Receiver<u16>) =
        mpsc::channel();

    let (interpreter_visual_tx, interpreter_visual_rx): (
        Sender<Vec<BuffItem>>,
        Receiver<Vec<BuffItem>>,
    ) = mpsc::channel();

    trace!(target: "main", "Created communication channels");

    /* Create and spawn the threads */

    trace!(target: "main", "Invoking threads");

    let socket_thread =
        thread::spawn(move || socket_thread(command_line.port, socket_interpreter_tx));

    let interpreter_thread = thread::spawn(move ||
        // Create a newton interpreter
        interpreter_thread(
            Interpreter::new(command_line.number_of_leds),
            socket_interpreter_rx,
            interpreter_visual_tx,
        ));

    // Join the threads
    trace!(target: "main", "All threads spawned");

    socket_thread.join().unwrap();
    interpreter_thread.join().unwrap();

    trace!(target: "main", "All threads joined");
}

fn socket_thread(port: u16, socket_sender: Sender<u16>) {
    trace!(target: "socket_thread", "Initialized thread");

    // Create the TCP listener
    let socket_addr = SocketAddr::from(([127, 0, 0, 1], port));
    let tcp_listener = TcpListener::bind(socket_addr)
        .expect(format!("Failed to bind for socket: {:?}", socket_addr).as_str());

    // Array of incoming connections
    for listener in tcp_listener.incoming() {
        match listener {
            Err(e) => error!(target: "socket_thread", "Error during socket connection: {:?}", e),
            Ok(tcp_stream) => {
                // Spawn client thread
                let socket_sender_moved = socket_sender.clone();
                thread::spawn(move || socket_handle_client(tcp_stream, socket_sender_moved));
            }
        }
    }
}

fn socket_handle_client(mut stream: TcpStream, socket_sender: Sender<u16>) {
    // Show connected peer
    info!(target: "socket_handle_client", "Handling peer `{}` on thread `{:?}`", stream.peer_addr().unwrap(), std::thread::current().id());

    // Read values from the buffer
    let mut read_buffer: [u8; 2] = [0, 0];
    while match stream.read_exact(&mut read_buffer) {
        Ok(_) => {
            // Parse into u16
            let mut bytes: u16 = 0x0;

            bytes |= (read_buffer[0] as u16) << 8;
            bytes |= read_buffer[1] as u16;

            match socket_sender.send(bytes) {
                Ok(_) => {
                    debug!(target: "socket_handle_client", "Tranferred value: 0x{:0004X}", bytes)
                }

                Err(e) => {
                    error!(target: "socket_handle_client", "Failed to transfer bytes to interpreter:\n{:?}", e)
                }
            };

            true
        }
        Err(_) => {
            error!(target: "socket_handle_client", "Closed connection with peer: {}", stream.peer_addr().unwrap());
            stream.shutdown(Shutdown::Both).unwrap();
            false
        }
    } {}
}

/// [Interpreter] thread, recieves an u16 from [socket_thread] and parses it as a [libnewton::core::instruction::Instruction]
fn interpreter_thread(
    mut newton_interpreter: Interpreter,
    socket_reciever: Receiver<u16>,
    visual_sender: Sender<Vec<BuffItem>>,
) {
    trace!(target: "interpreter_thread", "Initialized thread");

    loop {
        // Get the bytes from the socket
        let instruction_bytes = socket_reciever.recv().unwrap();
        // Parse the instruction
        let instruction_parsed = Instruction::parse_from_u16(instruction_bytes);

        match instruction_parsed {
            Err(error) => {
                error!(target: "interpreter_thread", "Instruction parsing error:\n{:?}", error)
            }
            Ok(instruction) => {
                // Interpret the instruction
                let execution_result = newton_interpreter.interpret(&instruction);

                match execution_result {
                    Err(newton_exception) => error!(target: "interpreter_thread",
                        "Newton interpreter generated an exception:\n{:?}",
                        newton_exception
                    ),

                    Ok(InterpreterAction::NoAction) => {
                        debug!(target: "interpreter_thread", "{:?} -> {:?}", instruction.instruction, instruction)
                    }

                    Ok(InterpreterAction::Sleep(sleep_time)) => {
                        info!(target: "interpreter_thread", "Sleep -> Stopping execution for {} ms", sleep_time);
                        std::thread::sleep(Duration::from_millis(sleep_time))
                    }

                    Ok(InterpreterAction::Update) => {
                        match visual_sender.send(newton_interpreter.public_buffer.clone()) {
                            Err(e) => {
                                error!(target: "interpreter_thread", "Error sending changes: {:?}", e)
                            }
                            Ok(_) => {
                                info!(target: "interpreter_thread", "Update -> Requested update")
                            }
                        }
                    }
                }
            }
        }
    }
}
