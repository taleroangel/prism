use crate::concurrency::BidirectionalChannel;

use std::{
    io::{Error, ErrorKind, Read, Write},
    net::{SocketAddr, TcpListener, TcpStream},
    sync::{
        atomic::{AtomicBool, Ordering},
        mpsc::TryRecvError,
        Arc,
    },
};

pub fn server_thread(
    port: u16,
    channel: BidirectionalChannel<Option<u16>>,
    continue_execution: Arc<AtomicBool>,
) {
    trace!(target: "server_thread", "Spawned thread");

    // Create the TCP listener
    let socket_addr = SocketAddr::from(([127, 0, 0, 1], port));
    let tcp_listener = TcpListener::bind(socket_addr)
        .expect(format!("Failed to bind for socket: {:?}", socket_addr).as_str());

    // Set as non blocking
    tcp_listener.set_nonblocking(true).unwrap();

    while continue_execution.load(Ordering::Relaxed) {
        match tcp_listener.accept() {
            Err(ref e) if e.kind() == ErrorKind::WouldBlock => {}
            Err(e) => {
                error!(target: "server_thread", "Exception on socket: {:#}", e);
                break;
            }
            Ok((stream, addr)) => {
                info!(target: "server_thread", "Accepted client: {}", addr);
                if server_handle(stream, &channel, continue_execution.clone()).is_err() {
                    error!(target: "server_thread", "Client finished with error");
                    break;
                }

                info!(target: "server_thread", "Now listening to new clients");
            }
        }
    }

    continue_execution.store(false, Ordering::Relaxed);
    info!(target: "server_thread", "Stopped listening for incoming connections");
}

fn server_handle(
    mut stream: TcpStream,
    channel: &BidirectionalChannel<Option<u16>>,
    continue_execution: Arc<AtomicBool>,
) -> Result<(), Error> {
    // Set as non-blocking client
    stream.set_nonblocking(false).unwrap();

    // Continue execution flag
    while continue_execution.load(Ordering::Relaxed) {
        // Read from the Stream
        let mut reading_buffer: [u8; 2] = [0, 0];

        match stream.read_exact(&mut reading_buffer) {
            Err(ref e) if e.kind() == ErrorKind::WouldBlock => {
                continue;
            }
            Err(ref e) if e.kind() == ErrorKind::UnexpectedEof => {
                // Finish with no error
                trace!(target: "server_handle", "Client aborted connection");
                return Ok(());
            }
            Err(e) => {
                error!(target: "server_handle", "Exception on socket ({:#}): {:#}", e.kind(), e);
                return Err(e);
            }
            Ok(_) => {
                // Where to store the bytes
                let mut bytes: u16 = 0x0;

                // Parse bytes to u16
                bytes |= (reading_buffer[0] as u16) << 8;
                bytes |= reading_buffer[1] as u16;

                // Send instruction to newton
                // ! Always send Some
                channel.tx.send(Some(bytes)).unwrap();
            }
        }

        // Read from Newton and write to Stream
        trace!(target: "server_handle", "Waiting for interpreter reply");
        while continue_execution.load(Ordering::Relaxed) {
            // Wait for responde from the interpreter
            match channel.rx.try_recv() {
                Err(TryRecvError::Empty) => continue,
                Err(TryRecvError::Disconnected) => {
                    error!(target: "server_handle", "Disconnected bidirectional channel");
                    return Err(Error::new(ErrorKind::BrokenPipe, "Disconnected channel"));
                }
                Ok(response) => {
                    match response {
                        None => {
                            trace!(target: "server_handle", "Reply from the interpreter")
                        }
                        Some(registers) => {
                            info!(target: "server_handle", "Writing registers: {:#}", registers);
                            // Parse values into buffer
                            reading_buffer = [(registers >> 8) as u8, (registers & 0x00FF) as u8];
                            // Write to the stream
                            stream.write_all(&reading_buffer).unwrap();
                        }
                    }

                    // Exit from the execution loop
                    break;
                }
            }
        }
    }

    // Finish with no error
    trace!(target: "server_handle", "Finishing execution");
    Ok(())
}
