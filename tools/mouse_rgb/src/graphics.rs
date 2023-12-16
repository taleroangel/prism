use colors_transform::{Color, Hsl};
use sdl2::{
    event::Event,
    mouse::MouseState,
    pixels,
    rect::{Point, Rect},
};
use std::{
    ops::{Add, Div, Mul, Sub},
    time::Duration,
};

const FRAMERATE_LIMITER: u64 = 10;
const EXTRA_HEIGHT: u32 = 100;

fn map_range<T: Copy>(s: T, from_range: (T, T), to_range: (T, T)) -> T
where
    T: Add<T, Output = T> + Sub<T, Output = T> + Mul<T, Output = T> + Div<T, Output = T>,
{
    to_range.0 + (s - from_range.0) * (to_range.1 - to_range.0) / (from_range.1 - from_range.0)
}

fn create_point_canvas(size: u32) -> Vec<Vec<pixels::Color>> {
    // Create the canvas point cloud
    let mut point_canvas = vec![vec![pixels::Color::RGB(255, 0, 0); size as usize]; size as usize];

    for x in 0..size {
        for y in 0..size {
            // Get the color mapped
            let mapped_hue: u32 = map_range(x, (0, size), (0, 360));
            let mapped_lightness = map_range(size - y, (0, size), (5, 95));
            let color = Hsl::from(mapped_hue as f32, 100.0, mapped_lightness as f32).to_rgb();

            // Set the color in the canvas
            point_canvas[x as usize][y as usize] = pixels::Color::RGB(
                color.get_red() as u8,
                color.get_green() as u8,
                color.get_blue() as u8,
            );
        }
    }

    point_canvas
}

pub fn graphics_thread(size: u32, producer: std::sync::mpsc::Sender<pixels::Color>) {
    // Create the canvas point cloud
    let point_canvas = create_point_canvas(size);

    let sdl_context = sdl2::init().unwrap();
    let video_subsystem = sdl_context.video().unwrap();

    let window = video_subsystem
        .window("Mouse RGB", size, size + EXTRA_HEIGHT)
        .position_centered()
        .build()
        .unwrap();

    let mut canvas = window.into_canvas().build().unwrap();
    let mut event_pump = sdl_context.event_pump().unwrap();

    'execution_loop: loop {
        /* Get events */
        for event in event_pump.poll_iter() {
            match event {
                Event::Quit { .. } => break 'execution_loop,
                _ => {}
            }
        }

        // Get current mouse state
        let mouse_state = MouseState::new(&event_pump);

        /* Clear render */
        canvas.set_draw_color(pixels::Color::RGB(0, 0, 0));
        canvas.clear();

        // Draw the RGB spectrum
        for x in 0..size {
            for y in 0..size {
                // Set the drawing color
                canvas.set_draw_color(point_canvas[x as usize][y as usize]);
                // Draw a pixel at the current position
                canvas.draw_point(Point::new(x as i32, y as i32)).unwrap();
            }
        }

        // Draw the current color
        if (mouse_state.x() as u32) < size && (mouse_state.y() as u32) < size {
            let current_color = point_canvas[mouse_state.x() as usize][mouse_state.y() as usize];
            canvas.set_draw_color(current_color);
            canvas
                .fill_rect(Rect::new(0, size as i32, size, EXTRA_HEIGHT))
                .unwrap();

            // Send the color to the consumer
            producer.send(current_color).unwrap();
        }

        /* Render present */
        canvas.present();

        /* Wait */
        std::thread::sleep(Duration::from_millis(FRAMERATE_LIMITER));
    }
}
