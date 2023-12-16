use crate::concurrency::BidirectionalChannel;

use std::{
    sync::{
        atomic::{AtomicBool, Ordering},
        mpsc::{Sender, TryRecvError},
        Arc, RwLock,
    },
    time::Duration,
};

use libnewton::{
    core::instruction::{Instruction, InstructionSet},
    interpreter::{BuffItem, Interpreter, InterpreterAction},
};

pub fn interpreter_thread(
    interpreter_lock: Arc<RwLock<Interpreter>>,
    server_channel: BidirectionalChannel<Option<u16>>,
    emulator_channel: Sender<Vec<BuffItem>>,
    continue_execution: Arc<AtomicBool>,
) {
    trace!(target: "interpreter_thread", "Spawned thread");

    // While stopping flag is false
    while continue_execution.load(Ordering::Relaxed) {
        // Get the bytes from the socket
        let instruction_bytes = match server_channel.rx.try_recv() {
            Ok(value) => value,
            Err(TryRecvError::Empty) => continue,
            Err(TryRecvError::Disconnected) => {
                warn!(target: "interpreter_thread", "socket_reciever closed connection");
                break;
            }
        }
        .unwrap();

        // Parse the instruction
        let instruction_parsed = Instruction::parse_from_u16(instruction_bytes);

        match instruction_parsed {
            Err(error) => {
                error!(target: "interpreter_thread", "Instruction parsing error:{:?}", error);
				break;
            }

            Ok(instruction) => {
                /* Newton interpreter 'write' lifetime */
                {
                    // Obtain lock on interpreter
                    let mut newton_interpreter = interpreter_lock.write().unwrap();

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
                            match emulator_channel.send(newton_interpreter.public_buffer.clone()) {
                                Err(e) => {
                                    error!(target: "interpreter_thread", "Error sending changes: {:?}", e)
                                }
                                Ok(_) => {
                                    info!(target: "interpreter_thread", "Update -> Requested update");
                                }
                            }
                        }
                    }
                }

                // Grab newton interpreter as read
                let newton_interpreter = interpreter_lock.read().unwrap();

                // Check if LDX or LDY returns a value
                match instruction.instruction {
                    // Return the variables
                    InstructionSet::LoadX | InstructionSet::LoadY => {
                        let registers = newton_interpreter.registers();
                        let variables: u16 = ((registers.x as u16) << 8) & (registers.y as u16);
                        server_channel.tx.send(Some(variables)).unwrap();
                    }

                    // Return none
                    _ => {
                        server_channel.tx.send(None).unwrap();
                    }
                }
            }
        }
    }

    // Finish the execution flag
    continue_execution.store(false, Ordering::Relaxed);
    info!(target: "interpreter_thread", "Newton interpreter stopped execution");
}
