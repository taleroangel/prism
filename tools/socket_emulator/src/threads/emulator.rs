use std::{
    sync::{
        atomic::{AtomicBool, Ordering},
        mpsc::{Receiver, TryRecvError},
        Arc,
    },
    time::Duration,
};

use libnewton::interpreter::BuffItem;
use sdl2::{event::Event, keyboard::Keycode, pixels::Color, rect::Rect};

const FRAMERATE_LIMITER: u64 = 15;
const BORDER_SIZE: u32 = 2;
const PADDING_SIZE: u32 = 10;

pub fn emulator_thread(
    size: (u32, u32),
    buffer_size: u8,
    interpreter_reciever: Receiver<Vec<BuffItem>>,
    continue_execution: Arc<AtomicBool>,
) {
    trace!(target: "emulator", "Spawned thread");

    let sdl_context = sdl2::init().unwrap();
    let video_subsystem = sdl_context.video().unwrap();
    let emulator_window = video_subsystem
        .window(
            "Prism Emulator",
            (size.0 + PADDING_SIZE) * (buffer_size as u32) + PADDING_SIZE,
            size.1 + (2 * PADDING_SIZE),
        )
        .position_centered()
        .build()
        .unwrap();

    trace!(target: "emulator", "Built SDL2 Context");

    let mut emulator_canvas = emulator_window.into_canvas().build().unwrap();
    let mut event_pump = sdl_context.event_pump().unwrap();

    // Create the display buffer
    let mut screen_buffer = vec![BuffItem::new(); buffer_size as usize];

    'displayloop: while continue_execution.load(Ordering::Relaxed) {
        /* Get events */
        for event in event_pump.poll_iter() {
            match event {
                Event::Quit { .. }
                | Event::KeyDown {
                    keycode: Some(Keycode::Escape),
                    ..
                } => {
                    info!(target: "emulator", "Closed window");
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
            Ok(value) => screen_buffer = value,
            Err(TryRecvError::Empty) => {}
            Err(TryRecvError::Disconnected) => {
                warn!(target: "emulator", "interpreter_reciever disconnected");
                break 'displayloop;
            }
        };

        for (index, pixel_value) in screen_buffer.iter().enumerate() {
            // Set the drawing color
            emulator_canvas.set_draw_color(Color::RGB(0xFF, 0xFF, 0xFF));

            // Set the pixel
            match emulator_canvas.fill_rect(Rect::new(
                (index as i32 * (size.0 + PADDING_SIZE) as i32)
                    + ((PADDING_SIZE - BORDER_SIZE / 2) as i32),
                (PADDING_SIZE - BORDER_SIZE / 2) as i32,
                size.0 + BORDER_SIZE,
                size.1 + BORDER_SIZE,
            )) {
                Ok(_) => {}
                Err(_) => {
                    error!(target: "emulator", "Failed to draw pixel at `{}` with value `{:?}`", index, pixel_value);
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
                (index as i32 * (size.0 + PADDING_SIZE) as i32) + PADDING_SIZE as i32,
                PADDING_SIZE as i32,
                size.0,
                size.1,
            )) {
                Ok(_) => {}
                Err(_) => {
                    error!(target: "emulator", "Failed to draw pixel at `{}` with value `{:?}`", index, pixel_value);
                    break 'displayloop;
                }
            }
        }

        /* Render present */
        emulator_canvas.present();

        /* Wait */
        std::thread::sleep(Duration::from_millis(FRAMERATE_LIMITER));
    }

    // Set the execution flag
    continue_execution.store(false, Ordering::Relaxed);
    trace!(target: "emulator", "Set stopping_flag to true")
}
