mod cli;
mod concurrency;
mod threads;

#[macro_use]
extern crate log;

use std::sync::{atomic::AtomicBool, Arc, RwLock};

use clap::Parser;

use crate::concurrency::BidirectionalChannel;

fn main() {
    pretty_env_logger::init();

    /* Parse command line arguments */
    let command_line = cli::CommandLine::parse();
    trace!(target: "main", "Initialized with arguments: {:?}", command_line);

    /* Prepare threads variable */

    // Create the execution flag
    let continue_execution = Arc::new(AtomicBool::new(true));

    // Create the newton interpreter
    let newton_interpreter = Arc::new(RwLock::new(libnewton::interpreter::Interpreter::new(
        command_line.number_of_leds,
    )));

    // Create the interpreter_channels
    let (server_channel, interpreter_channel) = BidirectionalChannel::new();
    let (interpreter_tx, emulator_rx) = std::sync::mpsc::channel();

    /* Create the threads */
    let execution_flag = continue_execution.clone();
    let interpreter_copy = newton_interpreter.clone();
    let interpreter_thread = std::thread::spawn(move || {
        threads::newton::interpreter_thread(
            interpreter_copy,
            interpreter_channel,
            interpreter_tx,
            execution_flag,
        );
    });

    let execution_flag = continue_execution.clone();
    std::thread::spawn(move || {
        threads::server::server_thread(command_line.port, server_channel, execution_flag);
    });

    let execution_flag = continue_execution.clone();
    let emulator_thread = std::thread::spawn(move || {
        threads::emulator::emulator_thread(
            (command_line.width, command_line.height),
            command_line.number_of_leds,
            emulator_rx,
            execution_flag,
        );
    });

    let execution_flag = continue_execution.clone();
    let interpreter_copy = newton_interpreter.clone();
    let debugger_thread = std::thread::spawn(move || {
        threads::debugger::debugger_thread(interpreter_copy, execution_flag);
    });

    /* Wait for threads */
    emulator_thread.join().unwrap();
    debugger_thread.join().unwrap();
    interpreter_thread.join().unwrap();
}
