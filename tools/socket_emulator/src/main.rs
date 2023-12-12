#[macro_use]
extern crate log;

use clap::Parser;
use libnewton::{
    core::instruction::Instruction,
    interpreter::{Interpreter, InterpreterAction},
};
use sdl2::{event::Event, keyboard::Keycode, pixels::Color, rect::Rect};

const FRAMERATE_LIMITER: u64 = 15;
const BORDER_SIZE: u32 = 2;

use std::{
    io::{ErrorKind, Read},
    net::{Shutdown, SocketAddr, TcpListener, TcpStream},
    sync::{
        atomic::{AtomicBool, Ordering},
        mpsc::{self, Receiver, Sender, TryRecvError},
        Arc,
    },
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
    #[arg(long, default_value_t = 100)]
    width: u32,
    #[arg(long, default_value_t = 100)]
    height: u32,
    #[arg(long, default_value_t = 10)]
    padding: u32,
    #[arg(long, default_value_t = 5)]
    text_size: u32,
}

struct SizeProperties {
    width: u32,
    height: u32,
    padding: u32,
    text_size: u32,
}

impl SizeProperties {
    fn new(command_line: &CommandLine) -> Self {
        Self {
            width: command_line.width,
            height: command_line.height,
            padding: command_line.padding,
            text_size: command_line.text_size,
        }
    }
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
        Sender<Interpreter>,
        Receiver<Interpreter>,
    ) = mpsc::channel();

    trace!(target: "main", "Created communication channels");

    /* Create and spawn the threads */

    let stopping_flag = Arc::new(AtomicBool::new(false));

    trace!(target: "main", "Invoking threads");

    let stopping_flag_copy = stopping_flag.clone();
    let socket_thread = thread::spawn(move || {
        socket_thread(command_line.port, socket_interpreter_tx, stopping_flag_copy)
    });

    let stopping_flag_copy = stopping_flag.clone();
    let interpreter_thread = thread::spawn(move ||
        // Create a newton interpreter
        interpreter_thread(
            Interpreter::new(command_line.number_of_leds),
            socket_interpreter_rx,
            interpreter_visual_tx,
			stopping_flag_copy,
        ));

    let stopping_flag_copy = stopping_flag.clone();
    let visual = thread::spawn(move || {
        // Call the thread
        visual_thread(
            SizeProperties::new(&command_line),
            command_line.number_of_leds,
            interpreter_visual_rx,
            stopping_flag_copy,
        );
    });

    // Join the threads
    trace!(target: "main", "All threads spawned");

    visual.join().unwrap();
    socket_thread.join().unwrap();
    interpreter_thread.join().unwrap();

    trace!(target: "main", "All threads joined");
}

fn socket_thread(port: u16, socket_sender: Sender<u16>, stopping_flag: Arc<AtomicBool>) {
    trace!(target: "socket_thread", "Spawned thread");

    // Create the TCP listener
    let socket_addr = SocketAddr::from(([127, 0, 0, 1], port));
    let tcp_listener = TcpListener::bind(socket_addr)
        .expect(format!("Failed to bind for socket: {:?}", socket_addr).as_str());

    // Set as non blocking
    tcp_listener.set_nonblocking(true).unwrap();

    // Array of incoming connections
    for listener in tcp_listener.incoming() {
        match listener {
            Err(ref e) if e.kind() == ErrorKind::WouldBlock => {}
            Err(e) => error!(target: "socket_thread", "Error during socket connection: {:?}", e),
            Ok(tcp_stream) => {
                // Spawn client thread
                let socket_sender_moved = socket_sender.clone();
                let stopping_flag_moved = stopping_flag.clone();
                thread::spawn(move || {
                    socket_handle_client(tcp_stream, socket_sender_moved, stopping_flag_moved)
                });
            }
        }

        // Stop execution if flag is set
        if stopping_flag.load(Ordering::Relaxed) {
            debug!(target: "socket_thread", "Thread recieved stopping signal");
            break;
        }
    }

    info!(target: "socket_thread", "Stopped listening for incoming connections");
}

fn socket_handle_client(
    mut stream: TcpStream,
    socket_sender: Sender<u16>,
    stopping_flag: Arc<AtomicBool>,
) {
    // Show connected peer
    info!(target: "socket_handle_client", "Handling peer `{}` on thread `{:?}`", stream.peer_addr().unwrap(), std::thread::current().id());

    // Read values from the buffer
    let mut read_buffer: [u8; 2] = [0, 0];
    while match stream.read_exact(&mut read_buffer) {
        Err(_) => false,
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
    } {
        // Stop execution if flag is set
        if stopping_flag.load(Ordering::Relaxed) {
            break;
        }
    }

    // Close the stream
    stream.shutdown(Shutdown::Both).unwrap();
    error!(target: "socket_handle_client", "Closed connection with peer: {}", stream.peer_addr().unwrap());
}

/// [Interpreter] thread, recieves an u16 from [socket_thread] and parses it as a [libnewton::core::instruction::Instruction]
fn interpreter_thread(
    mut newton_interpreter: Interpreter,
    socket_reciever: Receiver<u16>,
    visual_sender: Sender<Interpreter>,
    stopping_flag: Arc<AtomicBool>,
) {
    trace!(target: "interpreter_thread", "Spawned thread");

    // While stopping flag is false
    while !stopping_flag.load(Ordering::Relaxed) {
        // Get the bytes from the socket
        let instruction_bytes = match socket_reciever.try_recv() {
            Ok(value) => value,
            Err(TryRecvError::Empty) => continue,
            Err(TryRecvError::Disconnected) => {
                warn!(target: "interpreter_thread", "socket_reciever closed connection");
                break;
            }
        };

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
                        match visual_sender.send(newton_interpreter.clone()) {
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
        }
    }

    info!(target: "interpreter_thread", "Newton interpreter stopped execution");
}

fn visual_thread(
    size: SizeProperties,
    buffer_size: u8,
    interpreter_reciever: Receiver<Interpreter>,
    stopping_flag: Arc<AtomicBool>,
) {
    trace!(target: "visual_thread", "Spawned thread");

    let sdl_context = sdl2::init().unwrap();
    let video_subsystem = sdl_context.video().unwrap();
    let emulator_window = video_subsystem
        .window(
            "Prism Emulator",
            (size.width + size.padding) * (buffer_size as u32) + size.padding,
            size.height + (2 * size.padding) + size.text_size,
        )
        .position_centered()
        .opengl()
        .build()
        .unwrap();

    trace!(target: "visual_thread", "Built SDL2 Context");

    let find_sdl_gl_driver = || {
        for (index, item) in sdl2::render::drivers().enumerate() {
            if item.name == "opengl" {
                return Some(index as u32);
            }
        }
        None
    };

    let mut emulator_canvas = emulator_window
        .into_canvas()
        .index(find_sdl_gl_driver().unwrap())
        .build()
        .unwrap();

    let mut event_pump = sdl_context.event_pump().unwrap();

    // Create the display buffer
    let mut newton_interpreter = Interpreter::new(buffer_size);

    'displayloop: loop {
        /* Get events */
        for event in event_pump.poll_iter() {
            match event {
                Event::Quit { .. }
                | Event::KeyDown {
                    keycode: Some(Keycode::Escape),
                    ..
                } => {
                    info!(target: "visual_thread", "Closed window");
                    break 'displayloop;
                }
                _ => {}
            }
        }

        /* Clear render */
        emulator_canvas.set_draw_color(Color::RGB(0, 0, 0));
        emulator_canvas.clear();

        /* Render */
        // Get update values
        match interpreter_reciever.try_recv() {
            Ok(value) => newton_interpreter = value,
            Err(TryRecvError::Empty) => {}
            Err(TryRecvError::Disconnected) => {
                warn!(target: "visual_thread", "interpreter_reciever disconnected");
                break 'displayloop;
            }
        };

        for (index, pixel_value) in newton_interpreter.public_buffer.iter().enumerate() {
            // Set the drawing color
            emulator_canvas.set_draw_color(Color::RGB(0xFF, 0xFF, 0xFF));

            // Set the pixel
            match emulator_canvas.fill_rect(Rect::new(
                (index as i32 * (size.width + size.padding) as i32)
                    + ((size.padding - BORDER_SIZE / 2) as i32),
                (size.padding - BORDER_SIZE / 2) as i32,
                size.width + BORDER_SIZE,
                size.height + BORDER_SIZE,
            )) {
                Ok(_) => {}
                Err(_) => {
                    error!(target: "visual_thread", "Failed to draw pixel at `{}` with value `{:?}`", index, pixel_value);
                    break 'displayloop;
                }
            }

            // Set the drawing color
            emulator_canvas.set_draw_color(Color::RGB(
                pixel_value.red,
                pixel_value.green,
                pixel_value.blue,
            ));

            // Set the pixel
            match emulator_canvas.fill_rect(Rect::new(
                (index as i32 * (size.width + size.padding) as i32) + size.padding as i32,
                size.padding as i32,
                size.width,
                size.height,
            )) {
                Ok(_) => {}
                Err(_) => {
                    error!(target: "visual_thread", "Failed to draw pixel at `{}` with value `{:?}`", index, pixel_value);
                    break 'displayloop;
                }
            }
        }

        /* Render present */
        emulator_canvas.present();

        /* Wait */
        thread::sleep(Duration::from_millis(FRAMERATE_LIMITER));
    }

    // Set the execution flag
    stopping_flag.store(true, Ordering::Relaxed);
    trace!(target: "visual_thread", "Set stopping_flag to true")
}
