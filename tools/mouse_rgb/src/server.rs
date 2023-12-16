use std::{
    io::Write,
    net::{SocketAddr, TcpStream},
    sync::mpsc::Receiver,
};

use libnewton::core::{
    instruction::{Instruction, InstructionSet},
    options::{InstructionOptions, RangeOptions, ColorOptions},
};
use sdl2::pixels::Color;

fn instruction_to_buffer(instruction: &Instruction, buffer: &mut [u8]) {
    let bytes = instruction.write_to_u16();
    buffer[0] = ((bytes & 0xFF00) >> 8) as u8;
    buffer[1] = (bytes & 0x00FF) as u8;
}

pub fn server_thread(port: u16, consumer: Receiver<Color>) {
    let socket_addr = SocketAddr::from(([127, 0, 0, 1], port));
    let mut tcp_client = TcpStream::connect(socket_addr).expect("Failed to connect to socket");

    let mut command_buffer: [u8; 2] = [0, 0];

    // Send start 0
    let instruction = Instruction {
        instruction: InstructionSet::Range,
        options: InstructionOptions::Range(RangeOptions::RelativeStart),
        value: 0x00,
    };

    instruction_to_buffer(&instruction, &mut command_buffer);
    tcp_client.write_all(&command_buffer).unwrap();

    // Send end 0
    let instruction = Instruction {
        instruction: InstructionSet::Range,
        options: InstructionOptions::Range(RangeOptions::RelativeEnd),
        value: 0xFF,
    };

    instruction_to_buffer(&instruction, &mut command_buffer);
    tcp_client.write_all(&command_buffer).unwrap();

    loop {
        match consumer.recv() {
            Err(_) => break,
            Ok(color) => {
                // Send RED
                let instruction = Instruction {
                    instruction: InstructionSet::Fill,
                    options: InstructionOptions::Color(ColorOptions::Red),
                    value: color.r,
                };

                instruction_to_buffer(&instruction, &mut command_buffer);
                tcp_client.write_all(&command_buffer).unwrap();

				// Send Green
                let instruction = Instruction {
                    instruction: InstructionSet::Fill,
                    options: InstructionOptions::Color(ColorOptions::Green),
                    value: color.g,
                };

                instruction_to_buffer(&instruction, &mut command_buffer);
                tcp_client.write_all(&command_buffer).unwrap();

				// Send Blue
                let instruction = Instruction {
                    instruction: InstructionSet::Fill,
                    options: InstructionOptions::Color(ColorOptions::Blue),
                    value: color.b,
                };

                instruction_to_buffer(&instruction, &mut command_buffer);
                tcp_client.write_all(&command_buffer).unwrap();

				// Send Update
                let instruction = Instruction::update();

                instruction_to_buffer(&instruction, &mut command_buffer);
                tcp_client.write_all(&command_buffer).unwrap();
            }
        }
    }
}
